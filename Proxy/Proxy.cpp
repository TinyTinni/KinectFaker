#pragma once

#include <Windows.h>
#define NUIAPI /*__declspec(dllexport)*/ WINAPI
#include <NuiApi.h>

#include <fstream> //fstream
#include <utility> //move, foward
#include <vector>
#include <memory>
#include <string>
#include <functional> //connection id

#include <locale> //convert wchar_t to char in NuiGetSensorById
#include <codecvt> //as above

#include <tchar.h> //_T
#include <comutil.h> //convert from BSTR to _bstr_t

#include <KinectFileDef.pb.h>
#include "NuiSensor_Faker.h"

#include <json.hpp>

#define SPDLOG_NO_DATETIME
#include <spdlog/spdlog.h>

#include <system_error>
#include <outcome.hpp>
#include <type_traits>

namespace outcome = OUTCOME_V2_NAMESPACE;
namespace js = nlohmann;

struct FakeDevice
{
    const std::string name;
    const std::string filename;
    _bstr_t connectionId;

    //todo: cache device?
};


std::vector<std::unique_ptr<FakeDevice>> g_devices = {};
HMODULE kinectHndl = NULL; // handle to original Kinect10.dll module
INuiSensor_Faker* g_singleDevice = NULL; // device which is used in single device mode (without creating a INuiSensor)

// loggers
std::shared_ptr<spdlog::logger> g_log = nullptr;
std::shared_ptr<spdlog::logger> g_callLog = nullptr;

bool file_exists(const _bstr_t name)
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
        g_log->critical("Kinect Faker: Unknown Error while reading config file.");
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
        // For optimization.
        DisableThreadLibraryCalls(hDllHandle);

        // create loggers
        g_log = spdlog::stdout_color_mt("fk_proxy");
        g_log->set_level(spdlog::level::trace);
        
        g_callLog = spdlog::stdout_logger_mt("call");
        g_callLog->set_level(spdlog::level::off);
        
        g_log->trace("Init Kinect Proxy.");

        // get original functions
        std::basic_string<TCHAR> systemdir(GetSystemDirectory(nullptr, 0)-1, _T('\0'));
        if (!GetSystemDirectory(&systemdir[0], static_cast<UINT>(systemdir.size())+1u))
            return false;

        const _bstr_t kdll_path = _bstr_t(systemdir.c_str()) + _T("\\Kinect10.dll");

        kinectHndl = LoadLibraryEx(kdll_path, NULL, 0);

        // Cannot find original .dll. Return false until the proxy-dll was tested without installed Kinect.
        // Anyway, it is not a good idea to not install at least the Kinect Redist
        if (!kinectHndl)
            g_log->warn("Could not load original dll at: {}", static_cast<const char*>(kdll_path));
        else
            g_log->trace("Loaded original Kinect10.dll");

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

template<typename R, typename ...Args>
outcome::result<R> call_nui(const char* name, Args... a)
{
    if (!kinectHndl)
        return std::errc::host_unreachable;

    static std::unordered_map<const char*, FARPROC> cached_procs;

    g_callLog->trace("{} ()", name);
    typedef R(*func)(Args...);
    auto it = cached_procs.find(name);
    if (it == cached_procs.end())
        it = cached_procs.emplace(name, GetProcAddress(kinectHndl, name)).first;
    //enable /std:c++17
    //if constexpr( std::is_same<R,void>::value)
    //{
    //    reinterpret_cast<func>(it->second)(std::forward<Args...>(a)...);
    //    return outcome::success();
    //}
    //else
    {
        return reinterpret_cast<func>(it->second)(std::forward<Args>(a)...);
    }
}

//---------------------------------------------------------------------

HRESULT NUIAPI NuiGetSensorCount(
    int *pCount
)
{
    if (const auto r = call_nui<HRESULT>("NuiGetSensorCount", pCount))
    {
        if (FAILED(r.value()))
            return r.value();
    }
    else
    {
        *pCount = 0;
    }

    *pCount += static_cast<int>(g_devices.size());
    return S_OK;
}
HRESULT NUIAPI NuiCreateSensorByIndex(
    int index,
    INuiSensor **ppNuiSensor
)
{
    int pCount;
    if (const auto r = call_nui<HRESULT>("NuiGetSensorCount", &pCount))
    {
        if (index < pCount)
        {
            auto r = call_nui<HRESULT>("NuiCreateSensorByIndex", index, ppNuiSensor);
            return (r) ? r.value() : E_NUI_BADINDEX;
        }
    }
    else
        pCount = 0;
    index -= pCount;
    if (index >= g_devices.size())
        return E_NUI_BADINDEX;

    std::ifstream scene_file(g_devices[index]->filename, std::ios::binary);
    if (!scene_file.is_open())
        return E_NUI_BADINDEX;

    kif::Scene scene;
    if (!scene.ParseFromIstream(&scene_file))
        return E_OUTOFMEMORY;

    auto frames = scene.frames_size();
    *ppNuiSensor = new INuiSensor_Faker(std::move(scene), g_devices[index]->connectionId, index + pCount);
    return S_OK;
}
HRESULT NUIAPI NuiCameraElevationGetAngle(
    LONG *plAngleDegrees
)
{
    g_callLog->trace("{} (...)", "NuiCameraElevationGetAngle");
    auto r = call_nui<HRESULT>("NuiCameraElevationGetAngle", plAngleDegrees);
    return (r) ? r.value() : E_NOTIMPL;
}

HRESULT NUIAPI NuiCameraElevationSetAngle(
    LONG lAngleDegrees
)
{
    if (const auto r = call_nui<HRESULT>("NuiCameraElevationSetAngle", lAngleDegrees))
        return r.value();
    else
        return E_NOTIMPL;
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
    if (const auto r = call_nui<HRESULT>("NuiImageGetColorPixelCoordinatesFromDepthPixel", 
        eColorResolution, pcViewArea, lDepthX, lDepthY, usDepthValue, plColorX, plColorY))
        return r.value();
    else
        return E_NOTIMPL;
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
    if (const auto r = call_nui<HRESULT>("NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution",
        eColorResolution, eDepthResolution, pcViewArea, lDepthX, lDepthY, usDepthValue, plColorX, plColorY))
        return r.value();
    else
        return E_NOTIMPL;
}

HRESULT NUIAPI NuiImageStreamGetNextFrame(
    HANDLE hStream,
    DWORD dwMillisecondsToWait,
    const NUI_IMAGE_FRAME **ppcImageFrame
)
{
    if (const auto r = call_nui<HRESULT>("NuiImageStreamGetNextFrame", hStream, dwMillisecondsToWait, ppcImageFrame))
        return r.value();
    else
        return E_NOTIMPL;
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
    if (const auto r = call_nui<HRESULT>("NuiImageStreamOpen",
        eImageType, eResolution, dwImageFrameFlags, dwFrameLimit, hNextFrameEvent, phStreamHandle))
        return r.value();
    else
        return E_NOTIMPL;
}

HRESULT NUIAPI NuiImageStreamReleaseFrame(
    HANDLE hStream,
    const NUI_IMAGE_FRAME *pImageFrame
)
{
    if (const auto r = call_nui<HRESULT>("NuiImageStreamReleaseFrame", hStream, pImageFrame))
        return r.value();
    else
        return E_NOTIMPL;
}

HRESULT NUIAPI NuiInitialize(
    DWORD dwFlags
)
{
    g_callLog->trace("{} (dwFlags={})", "NuiInitialize", dwFlags);

    if (g_singleDevice != nullptr)
    {
        g_log->warn("Single Fake Kinect already initialized.");
        return E_NUI_ALREADY_INITIALIZED;
    }
    int sensors;
    NuiGetSensorCount(&sensors);
    return NuiCreateSensorByIndex(sensors, reinterpret_cast<INuiSensor**>(&g_singleDevice)); //create from first in kinect list
}

HRESULT NUIAPI NuiSetFrameEndEvent(
    HANDLE hEvent,
    DWORD dwFrameEventFlag
)
{
    if (const auto r = call_nui<HRESULT>("NuiSetFrameEndEvent", hEvent, dwFrameEventFlag))
        return r.value();
    else
        return E_NOTIMPL;
}

void NUIAPI NuiShutdown()
{
    g_callLog->trace("{} called", "NuiShutdown");

    if (g_singleDevice)
    {
        g_singleDevice->Release();
        g_singleDevice = nullptr;
    }
    //else
    //    call_nui<void>("NuiShutdown");
}

HRESULT NUIAPI NuiSkeletonGetNextFrame(
    DWORD dwMillisecondsToWait,
    NUI_SKELETON_FRAME *pSkeletonFrame
)
{
    g_callLog->trace("{} (...)", "NuiSkeletonGetNextFrame");

    if (!g_singleDevice)
    {
        g_log->trace("Cannot call {}. Device not initialized.", "NuiSkeletonGetNextFrame");
        return E_NUI_NOTCONNECTED;
    }

    return g_singleDevice->NuiSkeletonGetNextFrame(dwMillisecondsToWait,pSkeletonFrame);
}

HRESULT NUIAPI NuiSkeletonTrackingDisable()
{
    g_callLog->trace("{} ()", "NuiSkeletonTrackingDisable");

    if (!g_singleDevice)
    {
        g_log->trace("Cannot call {}. Device not initialized.", "NuiSkeletonTrackingDisable");
        return E_NUI_DEVICE_NOT_CONNECTED;
    }

    return g_singleDevice->NuiSkeletonTrackingDisable();
}

HRESULT NUIAPI NuiSkeletonTrackingEnable(
    HANDLE hNextFrameEvent,
    DWORD dwFlags
)
{
    g_callLog->trace("{} (dwFlags={})", "NuiSkeletonTrackingEnable", dwFlags);
    if (!g_singleDevice)
    {
        g_log->trace("Cannot call {}. Device not initialized.","NuiSkeletonTrackingEnable");
        return E_NUI_DEVICE_NOT_CONNECTED;
    }

    return g_singleDevice->NuiSkeletonTrackingEnable(hNextFrameEvent, dwFlags);
}

HRESULT NUIAPI NuiTransformSmooth(
    NUI_SKELETON_FRAME *pSkeletonFrame,
    const NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothingParams
)
{
    g_callLog->trace("{} (...)", "NuiTransformSmooth");
    if (!g_singleDevice)
    {
        g_log->trace("Cannot call {}. Device not initialized.", "NuiTransformSmooth");
        return E_NUI_DEVICE_NOT_CONNECTED;
    }

    return g_singleDevice->NuiTransformSmooth(pSkeletonFrame, pSmoothingParams);
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
    for (int i = 0; i < g_devices.size(); ++i)
    {
        const auto d = g_devices[i].get();
        int num_devices;
        NuiGetSensorCount(&num_devices);
        num_devices -= static_cast<int>(g_devices.size());
        if (wcscmp(d->connectionId,instance_id) == 0) //maybe use hashing
        {
            kif::Scene scene; 
            std::ifstream scene_file(d->filename, std::ios::binary);
            if (!scene_file.is_open())
                continue;
            if (!scene.ParseFromIstream(&scene_file))
                continue;

            auto frames = scene.frames_size();

            *ppNuiSensor = new INuiSensor_Faker(std::move(scene), d->connectionId, num_devices+i);
            return S_OK;
        }
    }
    //else if nothing was found
    if (const auto r = call_nui<HRESULT>("NuiCreateSensorById", strInstanceId, ppNuiSensor))
        return r.value();
    else
        return E_NUI_BADINDEX;
}

void NUIAPI NuiSetDeviceStatusCallback(
    NuiStatusProc callback,
    void *pUserData
)
{
    if (const auto r = call_nui<HRESULT>("NuiSetDeviceStatusCallback", callback, pUserData))
        return;
    else
        return g_log->warn("Cannot set Callback. Not implemented and Kinect runtime not present.");
}

HRESULT NUIAPI NuiImageStreamSetImageFrameFlags(
    HANDLE hStream,
    DWORD dwImageFrameFlags
)
{
    if (const auto r = call_nui<HRESULT>("NuiImageStreamSetImageFrameFlags", hStream, dwImageFrameFlags))
        return r.value();
    else
        return E_NOTIMPL;
}

HRESULT NUIAPI NuiImageStreamGetImageFrameFlags(
    HANDLE hStream,
    DWORD *pdwImageFrameFlags
)
{
    if (const auto r = call_nui<HRESULT>("NuiImageStreamGetImageFrameFlags", hStream, pdwImageFrameFlags))
        return r.value();
    else
        return E_NOTIMPL;
}

HRESULT NUIAPI NuiGetAudioSource(
    INuiAudioBeam **ppDmo
)
{
    if (const auto r = call_nui<HRESULT>("NuiGetAudioSource", ppDmo))
        return r.value();
    else
        return E_NOTIMPL;
}

HRESULT NUIAPI NuiSkeletonSetTrackedSkeletons(
    DWORD TrackingIDs[NUI_SKELETON_MAX_TRACKED_COUNT]
)
{
    if (const auto r = call_nui<HRESULT>("NuiSkeletonSetTrackedSkeletons", TrackingIDs))
        return r.value();
    else
        return E_NOTIMPL;
}

HRESULT __stdcall NuiSkeletonCalculateBoneOrientations(
    const NUI_SKELETON_DATA *pSkeletonData,
    NUI_SKELETON_BONE_ORIENTATION *pBoneOrientations
)
{
    if (const auto r = call_nui<HRESULT>("NuiSkeletonCalculateBoneOrientations", pSkeletonData, pBoneOrientations))
        return r.value();
    else
        return E_NOTIMPL;
}

HRESULT NUIAPI NuiCreateCoordinateMapperFromParameters(
    ULONG dataByteCount,
    void *pData,
    INuiCoordinateMapper **ppCoordinateMapper
)
{
    if (const auto r = call_nui<HRESULT>("NuiCreateCoordinateMapperFromParameters", dataByteCount, pData, ppCoordinateMapper))
        return r.value();
    else
        return E_NOTIMPL;
}

HRESULT NUIAPI NuiCreateDepthFilter(
    LPCWSTR filename,
    LPCSTR factoryEntryPoint,
    INuiDepthFilter **ppDepthFilter
)
{
    if (const auto r = call_nui<HRESULT>("NuiCreateDepthFilter", filename, factoryEntryPoint, ppDepthFilter))
        return r.value();
    else
        return E_NOTIMPL;
}
