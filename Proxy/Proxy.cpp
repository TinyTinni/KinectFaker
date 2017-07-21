#pragma once

#include <Windows.h>
#define NUIAPI /*__declspec(dllexport)*/ WINAPI
#include <NuiApi.h>

#include <fstream> //fstream
#include <utility> //move
#include <vector>
#include <memory>
#include <string>
#include <functional> //connection id

#include <locale> //convert wchar_t to char in NuiGetSensorById
#include <codecvt> //^^

#include <tchar.h> //_T
#include <comutil.h> //convert from BSTR to _bstr_t

#include <KinectFileDef.pb.h>
#include "NuiSensor_Faker.h"

#include <json.hpp>

#define SPDLOG_NO_DATETIME
#include <spdlog/spdlog.h>

namespace js = nlohmann;

struct FakeDevice
{
    const std::string name;
    const std::string filename;
    _bstr_t connectionId;

    //todo: cache device
};


std::vector<std::unique_ptr<FakeDevice>> g_devices = {};
HMODULE kinectHndl = NULL;

// loggers
std::shared_ptr<spdlog::logger> g_log = nullptr;
std::shared_ptr<spdlog::logger> g_callLog = nullptr;

bool file_exists(const char* name)
{
    const auto fileAttrib = GetFileAttributes(name);
    return (fileAttrib == INVALID_FILE_ATTRIBUTES || (fileAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool create_devices()
{
    const char* strFile = "fake_kinect.config"; 
    std::ifstream file(strFile);
    if (!file.is_open())
    {
        g_log->error("Could not find config file \"{}\".\n No fake Device configured.", strFile);
        return false;
    }
    js::json config;
    try
    {
        g_log->trace("Read configuration file {}", strFile);
        config << file; // throws std::exceptions on parse error

        const auto config_end = config.end();

        //-------------- Config loggers ---------------
        auto config_it = config.find("logger");
        if (config_it != config_end)
        {
            const auto end = config_it->end();

            auto it = config_it->find("level");
            if (it != end)
                g_log->set_level(static_cast<spdlog::level::level_enum>(it->get<unsigned>()));
            else
                g_log->warn("Could not set logging level. Default to {}", g_log->level());

            it = config_it->find("trace_calls");
            if (it != end)
                g_callLog->set_level((it->get<bool>()) ? spdlog::level::trace : spdlog::level::off);

        }
        else
            g_log->warn("Loggers are not configured. Using Defaults.");

        g_log->trace("Call Log level: {} ({})", g_callLog->level(), spdlog::level::to_str(g_callLog->level()));
        g_log->trace("Log level: {} ({})", g_log->level(), spdlog::level::to_str(g_log->level()));


        //-------- Load Devices -------------
        config_it = config.find("devices");
        if (config_it != config_end)
        {
            for (auto it = config_it->cbegin(); it != config_it->cend(); ++it)
            {
                const auto& dev = it.value();
                const auto& dev_name = it.key();

                // connection_id
                const auto connID_it = dev.find("connection_id");
                _bstr_t connID = "";
                if (connID_it == dev.end())
                    g_log->warn("Could not find \"connection_id\" in configuartion");
                else
                    connID = connID_it.value().get<std::string>().c_str();

                // skeleton_file
                const auto skeleton_file_it = dev.find("skeleton_file");
                if (skeleton_file_it == dev.end())
                {
                    g_log->critical("Could not find skeleton_file in configuration. Can not create Fake Device \"{}\"", dev_name);
                    continue;
                }
                const auto skeleton_file = skeleton_file_it.value().get<std::string>();
                if (file_exists(skeleton_file.c_str()))
                    g_log->warn("File given by \"skeleton_file\" does not exist.\n skeleton_file : {}", skeleton_file);

                // device creation
                g_devices.push_back( //throws out_of_memory
                    std::make_unique<FakeDevice>(
                        FakeDevice{
                    dev_name,
                    skeleton_file, //skeleton file
                    connID //connectionId
                }
                ));
                g_log->trace("created new fake device: {}.", dev_name);
            }
        }
        else
            g_log->warn("No fake device created. Could not find any \"devices\" field.");

        g_log->trace("total fake devices: {}", g_devices.size());
    }
    catch (const std::exception& exp)
    {
        g_log->error(exp.what());
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
    {
        //  For optimization.
        DisableThreadLibraryCalls(hDllHandle);
        std::basic_string<TCHAR> systemdir(GetSystemDirectory(nullptr, 0)-1, _T('\0'));
        if (!GetSystemDirectory(&systemdir[0], (UINT)systemdir.size()+1))
            return false;

        const auto kdll_path = systemdir + _T("\\Kinect10.dll");

        kinectHndl = LoadLibraryEx(kdll_path.c_str(), NULL, 0);

        // Cannot find original .dll. Return false until the proxy-dll was tested without installed Kinect.
        // Anyway, it is not a good idea to not install at least the Kinect Redist
        if (!kinectHndl)
            return false;


        if (!g_log)
        {
            g_log = spdlog::stdout_color_mt("fk_proxy");
            g_log->set_level(spdlog::level::trace);
        }
        if (!g_callLog)
        {
            g_callLog = spdlog::stdout_logger_mt("call");
            g_callLog->set_level(spdlog::level::off);
        }
        g_log->trace("Init Kinect Proxy.");

        // Don't return false. Even if there are parsing errors or warnings,
        // library should be able redirect to the original Kinect10.dll even without Fake Devices
        if (!create_devices())
            return true;
    }
        break;
    case DLL_PROCESS_DETACH:
        //cleanup
        if (kinectHndl) FreeLibrary(kinectHndl);

        break;
    }

    

    return true;

}

//---------------------------------------------------------------------

HRESULT NUIAPI NuiGetSensorCount(
    int *pCount
)
{
    g_callLog->trace("{} ()", "NuiGetSensorCount");
    typedef HRESULT(*func)(int*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiGetSensorCount");
    HRESULT r = reinterpret_cast<func>(f)(pCount);

    if (FAILED(r))
        return r;
    *pCount += static_cast<int>(g_devices.size());
    return S_OK;
}
HRESULT NUIAPI NuiCreateSensorByIndex(
    int index,
    INuiSensor **ppNuiSensor
)
{
    g_callLog->trace("{} (index={})", "NuiCreateSensorByIndex", index);
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
    if (index >= g_devices.size())
        return ERROR_INVALID_INDEX; //msdn doc is wrong with returning values for this function. Check correct values by testing...

    std::ifstream scene_file(g_devices[index]->filename, std::ios::binary);
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
    g_callLog->trace("{} (...)", "NuiCameraElevationGetAngle");
    typedef HRESULT(*func)(LONG*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiCameraElevationGetAngle");
    return reinterpret_cast<func>(f)(plAngleDegrees);
}

HRESULT NUIAPI NuiCameraElevationSetAngle(
    LONG lAngleDegrees
)
{
    g_callLog->trace("{} (lAngleDegrees={})", "NuiCameraElevationSetAngle", lAngleDegrees);
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
    g_callLog->trace("{} (...)", "NuiImageGetColorPixelCoordinatesFromDepthPixel");
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
    g_callLog->trace("{} (...)", "NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution");
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
    g_callLog->trace("{} (...)", "NuiImageStreamGetNextFrame");
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
    g_callLog->trace("{} (...)", "NuiImageStreamOpen");
    typedef HRESULT(*func)(NUI_IMAGE_TYPE, NUI_IMAGE_RESOLUTION, DWORD, DWORD, HANDLE, HANDLE*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiImageStreamOpen");
    return reinterpret_cast<func>(f)(eImageType, eResolution, dwImageFrameFlags, dwFrameLimit, hNextFrameEvent, phStreamHandle);
}

HRESULT NUIAPI NuiImageStreamReleaseFrame(
    HANDLE hStream,
    const NUI_IMAGE_FRAME *pImageFrame
)
{
    g_callLog->trace("{} (...)", "NuiImageStreamReleaseFrame");
    typedef HRESULT(*func)(HANDLE, const NUI_IMAGE_FRAME*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiImageStreamReleaseFrame");
    return reinterpret_cast<func>(f)(hStream, pImageFrame);
}

HRESULT NUIAPI NuiInitialize(
    DWORD dwFlags
)
{
    g_callLog->trace("{} (dwFlags={})", "NuiInitialize", dwFlags);
    typedef HRESULT(*func)(DWORD);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiInitialize");
    return reinterpret_cast<func>(f)(dwFlags);
}

HRESULT NUIAPI NuiSetFrameEndEvent(
    HANDLE hEvent,
    DWORD dwFrameEventFlag
)
{
    g_callLog->trace("{} (...)", "NuiSetFrameEndEvent");
    typedef HRESULT(*func)(HANDLE, DWORD);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSetFrameEndEvent");
    return reinterpret_cast<func>(f)(hEvent, dwFrameEventFlag);
}

void NUIAPI NuiShutdown()
{
    g_callLog->trace("{} called", "NuiShutdown");
    typedef void(*func)();
    static FARPROC f = GetProcAddress(kinectHndl, "NuiShutdown");
    reinterpret_cast<func>(f)();
}

HRESULT NUIAPI NuiSkeletonGetNextFrame(
    DWORD dwMillisecondsToWait,
    NUI_SKELETON_FRAME *pSkeletonFrame
)
{
    g_callLog->trace("{} (...)", "NuiSkeletonGetNextFrame");
    typedef HRESULT(*func)(DWORD, NUI_SKELETON_FRAME*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSkeletonGetNextFrame");
    return reinterpret_cast<func>(f)(dwMillisecondsToWait, pSkeletonFrame);
}

HRESULT NUIAPI NuiSkeletonTrackingDisable()
{
    g_callLog->trace("{} ()", "NuiSkeletonTrackingDisable");
    typedef HRESULT(*func)();
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSkeletonTrackingDisable");
    return reinterpret_cast<func>(f)();
}

HRESULT NUIAPI NuiSkeletonTrackingEnable(
    HANDLE hNextFrameEvent,
    DWORD dwFlags
)
{
    g_callLog->trace("{} (dwFlags={})", "NuiSkeletonTrackingEnable", dwFlags);
    typedef HRESULT(*func)(HANDLE, DWORD);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSkeletonTrackingEnable");
    return reinterpret_cast<func>(f)(hNextFrameEvent, dwFlags);
}

HRESULT NUIAPI NuiTransformSmooth(
    NUI_SKELETON_FRAME *pSkeletonFrame,
    const NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothingParams
)
{
    g_callLog->trace("{} (...)", "NuiTransformSmooth");
    typedef HRESULT(*func)(NUI_SKELETON_FRAME*, const NUI_TRANSFORM_SMOOTH_PARAMETERS*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiTransformSmooth");
    return reinterpret_cast<func>(f)(pSkeletonFrame, pSmoothingParams);
}

HRESULT NUIAPI NuiCreateSensorById(
    const OLECHAR *strInstanceId,
    INuiSensor **ppNuiSensor
)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
    std::string instId = conv1.to_bytes(strInstanceId);
    g_callLog->trace("{} (strInstanceId={})", "NuiCreateSensorById", instId);
    // search for sensor with the given id
    _bstr_t instance_id = strInstanceId;
    for (const auto& d : g_devices)
    {
        if (wcscmp(d->connectionId,instance_id) == 0) //maybe use hashing
        {
            kif::Scene scene; 
            std::ifstream scene_file(d->filename, std::ios::binary);
            if (!scene_file.is_open())
                continue;
            if (!scene.ParseFromIstream(&scene_file))
                continue;

            auto frames = scene.frames_size();
            *ppNuiSensor = new INuiSensor_Faker(std::move(scene));
            return S_OK;
        }
    }
    //else if nothing was found

    typedef HRESULT(*func)(const OLECHAR*, INuiSensor**);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiCreateSensorById");
    return reinterpret_cast<func>(f)(strInstanceId, ppNuiSensor);
}

void NUIAPI NuiSetDeviceStatusCallback(
    NuiStatusProc callback,
    void *pUserData
)
{
    g_callLog->trace("{} (...)", "NuiSetDeviceStatusCallback");
    typedef void(*func)(NuiStatusProc, void*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSetDeviceStatusCallback");
    reinterpret_cast<func>(f)(callback, pUserData);
}

HRESULT NUIAPI NuiImageStreamSetImageFrameFlags(
    HANDLE hStream,
    DWORD dwImageFrameFlags
)
{
    g_callLog->trace("{} (dwImageFrameFlags={})", "NuiImageStreamSetImageFrameFlags", dwImageFrameFlags);
    typedef HRESULT(*func)(HANDLE, DWORD);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiImageStreamSetImageFrameFlags");
    return reinterpret_cast<func>(f)(hStream, dwImageFrameFlags);
}

HRESULT NUIAPI NuiImageStreamGetImageFrameFlags(
    HANDLE hStream,
    DWORD *pdwImageFrameFlags
)
{
    g_callLog->trace("{} (...)", "NuiImageStreamGetImageFrameFlags");
    typedef HRESULT(*func)(HANDLE, DWORD*);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiImageStreamGetImageFrameFlags");
    return reinterpret_cast<func>(f)(hStream, pdwImageFrameFlags);
}

HRESULT NUIAPI NuiGetAudioSource(
    INuiAudioBeam **ppDmo
)
{
    g_callLog->trace("{} (...)", "NuiGetAudioSource");
    typedef HRESULT(*func)(INuiAudioBeam**);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiGetAudioSource");
    return reinterpret_cast<func>(f)(ppDmo);
}

HRESULT NUIAPI NuiSkeletonSetTrackedSkeletons(
    DWORD TrackingIDs[NUI_SKELETON_MAX_TRACKED_COUNT]
)
{
    g_callLog->trace("{} (...)", "NuiSkeletonSetTrackedSkeletons");
    typedef HRESULT(*func)(DWORD[NUI_SKELETON_MAX_TRACKED_COUNT]);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiSkeletonSetTrackedSkeletons");
    return reinterpret_cast<func>(f)(TrackingIDs);
}

HRESULT __stdcall NuiSkeletonCalculateBoneOrientations(
    const NUI_SKELETON_DATA *pSkeletonData,
    NUI_SKELETON_BONE_ORIENTATION *pBoneOrientations
)
{
    g_callLog->trace("{} (...)", "NuiSkeletonCalculateBoneOrientations");
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
    g_callLog->trace("{} (...)", "NuiCreateCoordinateMapperFromParameters");
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
    g_callLog->trace("{} (...)", "NuiCreateDepthFilter");
    typedef HRESULT(*func)(LPCWSTR, LPCSTR, INuiDepthFilter**);
    static FARPROC f = GetProcAddress(kinectHndl, "NuiCreateDepthFilter");
    return reinterpret_cast<func>(f)(filename, factoryEntryPoint, ppDepthFilter);
}
