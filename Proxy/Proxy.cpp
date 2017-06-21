#pragma once

#include <Windows.h>
#define NUIAPI /*__declspec(dllexport)*/ WINAPI
#include <NuiApi.h>

#include <fstream> //fstream
#include <utility> //move
#include <vector>
#include <memory>
#include <string>

#include <KinectFileDef.pb.h>
#include "NuiSensor_Faker.h"

#include <json.hpp>
namespace js = nlohmann;

struct FakeDevice
{
    const std::string name;
    const std::string filename;
    const size_t idx;
};


std::vector<std::unique_ptr<FakeDevice>> currentDevices;
HMODULE kinectHndl = NULL;

bool create_devices()
{
    const char* strFile = "fake_kinect.config"; 
    std::ifstream file(strFile);
    if (!file.is_open())
        return true; // todo: logging
    js::json config;

    try
    {
        config << file; // throws std::exceptions on parse error

        const auto& devices = config.at("devices"); //throws out_of_range

        for (auto it = std::cbegin(devices); it != std::cend(devices); ++it)
        {
            const auto& dev = it.value();
            currentDevices.push_back( //throws out_of_memory
                std::make_unique<FakeDevice>(
                    FakeDevice{ it.key(),
                dev["skeleton_file"], //skeleton file
                currentDevices.size() //device id
            }
            ));
        }
    }
    catch (const std::out_of_range& exp)
    {
        std::cerr << "Warning: No Kinect Fake Device Found.";
        return false;
    }
    catch (const std::exception& exp)
    {
        std::cerr << "Kinect Faker Error: " << exp.what() << "\n";
        return false;
    }
    catch (...)
    {
        std::cerr << "Kinect Faker: Unknown Error.\n";
        return false;
    }

    return true;
}

BOOLEAN WINAPI DllMain(IN HINSTANCE hDllHandle,
    IN DWORD     nReason,
    IN LPVOID    Reserved)
{
    BOOLEAN bSuccess = TRUE;
    //  Perform global initialization.
    switch (nReason)
    {
    case DLL_PROCESS_ATTACH:
        //  For optimization.
        DisableThreadLibraryCalls(hDllHandle);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }

    kinectHndl = LoadLibraryEx("C:\\Windows\\System32\\Kinect10.dll",NULL,0);

    // Cannot find original .dll. Return false until the proxy-dll was tested without installed Kinect.
    // Anyway, it is not a good idea to not install at least the Kinect Redist
    if (!kinectHndl)
        return false;


    // Don't return false. Even if there are parsing errors or warnings,
    // library should be able redirect to the original Kinect10.dll even without Fake Devices
    if (!create_devices())
        return true;

    return true;

}

//---------------------------------------------------------------------

HRESULT NUIAPI NuiGetSensorCount(
    int *pCount
)
{
    typedef HRESULT(*func)(int*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiGetSensorCount");
    HRESULT r = reinterpret_cast<func>(f)(pCount);

    if (FAILED(r))
        return r;
    *pCount += static_cast<int>(currentDevices.size());
    return S_OK;
}
HRESULT NUIAPI NuiCreateSensorByIndex(
    int index,
    INuiSensor **ppNuiSensor
)
{
    int pCount;
    typedef HRESULT(*sensorCount)(int*);
    static FARPROC sc = GetProcAddress(kinectHndl, "NuiGetSensorCount");
    HRESULT r = reinterpret_cast<sensorCount>(sc)(&pCount);

    if (index < pCount)
    {
        typedef HRESULT(*func)(int, INuiSensor**);
        static FARPROC f = GetProcAddress(kinectHndl, "NuiCreateSensorByIndex");
        return reinterpret_cast<func>(f)(index, ppNuiSensor);
    }
    index -= pCount;
    if (index >= currentDevices.size())
        return ERROR_INVALID_INDEX; //msdn doc is wrong with returning values for this function. Check correct values by testing...

    std::ifstream scene_file(currentDevices[index]->filename, std::ios::binary);
    if (!scene_file.is_open())
        return STG_E_FILENOTFOUND;

    kif::Scene scene;
    if (!scene.ParseFromIstream(&scene_file))
        return E_OUTOFMEMORY;

    auto frames = scene.frames_size();
    *ppNuiSensor = new INuiSensor_Faker(std::move(scene));
    return S_OK;


}
HRESULT NUIAPI NuiCameraElevationGetAngle(
    LONG *plAngleDegrees
)
{
    typedef HRESULT(*func)(LONG*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiCameraElevationGetAngle");
    return reinterpret_cast<func>(f)(plAngleDegrees);
}

HRESULT NUIAPI NuiCameraElevationSetAngle(
    LONG lAngleDegrees
)
{
    typedef HRESULT(*func)(LONG);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiCameraElevationSetAngle");
    return reinterpret_cast<func>(f)(lAngleDegrees);
}

HRESULT NUIAPI NuiImageGetColorPixelCoordinatesFromDepthPixel(
    NUI_IMAGE_RESOLUTION eColorResolution,
    const NUI_IMAGE_VIEW_AREA *pcViewArea,
    LONG lDepthX,
    LONG lDepthY,
    USHORT usDepthValue,
    LONG *plColorX,
    LONG *plColorY
)
{
    typedef HRESULT(*func)(NUI_IMAGE_RESOLUTION, const NUI_IMAGE_VIEW_AREA*, LONG, LONG, USHORT, LONG*, LONG*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiImageGetColorPixelCoordinatesFromDepthPixel");
    return reinterpret_cast<func>(f)(eColorResolution, pcViewArea, lDepthX, lDepthY, usDepthValue, plColorX, plColorY);
}

HRESULT NUIAPI NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
    NUI_IMAGE_RESOLUTION eColorResolution,
    NUI_IMAGE_RESOLUTION eDepthResolution,
    const NUI_IMAGE_VIEW_AREA *pcViewArea,
    LONG lDepthX,
    LONG lDepthY,
    USHORT usDepthValue,
    LONG *plColorX,
    LONG *plColorY
)
{
    typedef HRESULT(*func)(NUI_IMAGE_RESOLUTION, NUI_IMAGE_RESOLUTION, const NUI_IMAGE_VIEW_AREA*, LONG, LONG, USHORT, LONG*, LONG*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution");
    return reinterpret_cast<func>(f)(eColorResolution, eDepthResolution, pcViewArea, lDepthX, lDepthY, usDepthValue, plColorX, plColorY);
}

HRESULT NUIAPI NuiImageStreamGetNextFrame(
    HANDLE hStream,
    DWORD dwMillisecondsToWait,
    const NUI_IMAGE_FRAME **ppcImageFrame
)
{
    typedef HRESULT(*func)(HANDLE, DWORD, const NUI_IMAGE_FRAME**);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiImageStreamGetNextFrame");
    return reinterpret_cast<func>(f)(hStream, dwMillisecondsToWait, ppcImageFrame);
}

HRESULT NUIAPI NuiImageStreamOpen(
    NUI_IMAGE_TYPE eImageType,
    NUI_IMAGE_RESOLUTION eResolution,
    DWORD dwImageFrameFlags,
    DWORD dwFrameLimit,
    HANDLE hNextFrameEvent,
    HANDLE *phStreamHandle
)
{
    typedef HRESULT(*func)(NUI_IMAGE_TYPE, NUI_IMAGE_RESOLUTION, DWORD, DWORD, HANDLE, HANDLE*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiImageStreamOpen");
    return reinterpret_cast<func>(f)(eImageType, eResolution, dwImageFrameFlags, dwFrameLimit, hNextFrameEvent, phStreamHandle);
}

HRESULT NUIAPI NuiImageStreamReleaseFrame(
    HANDLE hStream,
    const NUI_IMAGE_FRAME *pImageFrame
)
{
    typedef HRESULT(*func)(HANDLE, const NUI_IMAGE_FRAME*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiImageStreamReleaseFrame");
    return reinterpret_cast<func>(f)(hStream, pImageFrame);
}

HRESULT NUIAPI NuiInitialize(
    DWORD dwFlags
)
{
    typedef HRESULT(*func)(DWORD);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiInitialize");
    return reinterpret_cast<func>(f)(dwFlags);
}

HRESULT NUIAPI NuiSetFrameEndEvent(
    HANDLE hEvent,
    DWORD dwFrameEventFlag
)
{
    typedef HRESULT(*func)(HANDLE, DWORD);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSetFrameEndEvent");
    return reinterpret_cast<func>(f)(hEvent, dwFrameEventFlag);
}

void NUIAPI NuiShutdown()
{

    typedef void(*func)();
    static FARPROC f = GetProcAddress(kinectHndl, "NuiShutdown");
    reinterpret_cast<func>(f)();
}

HRESULT NUIAPI NuiSkeletonGetNextFrame(
    DWORD dwMillisecondsToWait,
    NUI_SKELETON_FRAME *pSkeletonFrame
)
{
    typedef HRESULT(*func)(DWORD, NUI_SKELETON_FRAME*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSkeletonGetNextFrame");
    return reinterpret_cast<func>(f)(dwMillisecondsToWait, pSkeletonFrame);
}

HRESULT NUIAPI NuiSkeletonTrackingDisable()
{
    typedef HRESULT(*func)();
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSkeletonTrackingDisable");
    return reinterpret_cast<func>(f)();
}

HRESULT NUIAPI NuiSkeletonTrackingEnable(
    HANDLE hNextFrameEvent,
    DWORD dwFlags
)
{
    typedef HRESULT(*func)(HANDLE, DWORD);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSkeletonTrackingEnable");
    return reinterpret_cast<func>(f)(hNextFrameEvent, dwFlags);
}

HRESULT NUIAPI NuiTransformSmooth(
    NUI_SKELETON_FRAME *pSkeletonFrame,
    const NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothingParams
)
{
    typedef HRESULT(*func)(NUI_SKELETON_FRAME*, const NUI_TRANSFORM_SMOOTH_PARAMETERS*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiTransformSmooth");
    return reinterpret_cast<func>(f)(pSkeletonFrame, pSmoothingParams);
}

HRESULT NUIAPI NuiCreateSensorById(
    const OLECHAR *strInstanceId,
    INuiSensor **ppNuiSensor
)
{
    typedef HRESULT(*func)(const OLECHAR*, INuiSensor**);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiCreateSensorById");
    return reinterpret_cast<func>(f)(strInstanceId, ppNuiSensor);
}

void NUIAPI NuiSetDeviceStatusCallback(
    NuiStatusProc callback,
    void *pUserData
)
{
    typedef void(*func)(NuiStatusProc, void*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSetDeviceStatusCallback");
    reinterpret_cast<func>(f)(callback, pUserData);
}

HRESULT NUIAPI NuiImageStreamSetImageFrameFlags(
    HANDLE hStream,
    DWORD dwImageFrameFlags
)
{
    typedef HRESULT(*func)(HANDLE, DWORD);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiImageStreamSetImageFrameFlags");
    return reinterpret_cast<func>(f)(hStream, dwImageFrameFlags);
}

HRESULT NUIAPI NuiImageStreamGetImageFrameFlags(
    HANDLE hStream,
    DWORD *pdwImageFrameFlags
)
{
    typedef HRESULT(*func)(HANDLE, DWORD*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiImageStreamGetImageFrameFlags");
    return reinterpret_cast<func>(f)(hStream, pdwImageFrameFlags);
}

HRESULT NUIAPI NuiGetAudioSource(
    INuiAudioBeam **ppDmo
)
{
    typedef HRESULT(*func)(INuiAudioBeam**);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiGetAudioSource");
    return reinterpret_cast<func>(f)(ppDmo);
}

HRESULT NUIAPI NuiSkeletonSetTrackedSkeletons(
    DWORD TrackingIDs[NUI_SKELETON_MAX_TRACKED_COUNT]
)
{
    typedef HRESULT(*func)(DWORD[NUI_SKELETON_MAX_TRACKED_COUNT]);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSkeletonSetTrackedSkeletons");
    return reinterpret_cast<func>(f)(TrackingIDs);
}

HRESULT __stdcall NuiSkeletonCalculateBoneOrientations(
    const NUI_SKELETON_DATA *pSkeletonData,
    NUI_SKELETON_BONE_ORIENTATION *pBoneOrientations
)
{
    typedef HRESULT(*func)(const NUI_SKELETON_DATA*, NUI_SKELETON_BONE_ORIENTATION*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSkeletonCalculateBoneOrientations");
    return reinterpret_cast<func>(f)(pSkeletonData, pBoneOrientations);
}

HRESULT NUIAPI NuiCreateCoordinateMapperFromParameters(
    ULONG dataByteCount,
    void *pData,
    INuiCoordinateMapper **ppCoordinateMapper
)
{
    typedef HRESULT(*func)(ULONG, void*, INuiCoordinateMapper**);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiCreateCoordinateMapperFromParameters");
    return reinterpret_cast<func>(f)(dataByteCount, pData, ppCoordinateMapper);
}

HRESULT NUIAPI NuiCreateDepthFilter(
    LPCWSTR filename,
    LPCSTR factoryEntryPoint,
    INuiDepthFilter **ppDepthFilter
)
{
    typedef HRESULT(*func)(LPCWSTR, LPCSTR, INuiDepthFilter**);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiCreateDepthFilter");
    return reinterpret_cast<func>(f)(filename, factoryEntryPoint, ppDepthFilter);
}
