#pragma once

#include <Windows.h>

#define NUIAPI /*__declspec(dllexport)*/ WINAPI
#include <NuiApi.h>

#include <fstream>
#include <utility> //move, foward
#include <vector>
#include <memory>
#include <string>
#include <functional> // invoke
#include <new> // nothrow

#include <locale> //convert wchar_t to char in NuiGetSensorById
#include <codecvt> //as above

#include <tchar.h> //_T
#include <comutil.h> //convert from BSTR to _bstr_t

#include "NuiSensor_Faker.h"
#include "NuiSensor_TracerAdaptor.h"

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
    const std::string imageFilename;
    std::shared_ptr<spdlog::logger> tracer;
    _bstr_t connectionId;
};


std::vector<std::unique_ptr<FakeDevice>> g_devices = {};
HMODULE kinectHndl = NULL; // handle to original Kinect10.dll module, is NULL, if Module was not found
bool is_proxy_init = false; // is false, if the proxy could not initialized e.g. due to error in configuration
INuiSensor_Faker* g_singleDevice = NULL; // device which is used in single device mode (without creating a INuiSensor)

// loggers
std::shared_ptr<spdlog::logger> g_log = nullptr;
std::shared_ptr<spdlog::logger> g_logTrace = nullptr;


bool file_exists(const _bstr_t name) noexcept
{
    const auto fileAttrib = GetFileAttributes(name);
    return !((fileAttrib == INVALID_FILE_ATTRIBUTES || (fileAttrib & FILE_ATTRIBUTE_DIRECTORY)));
}

std::error_code create_devices() noexcept
{
    const char* strFile = "fake_kinect.config"; 
    std::ifstream file(strFile);
    if (!file.is_open())
    {
        g_log->error("Could not find config file \"{}\".\n No fake Device configured.", strFile);
        return std::make_error_code(std::errc::no_such_file_or_directory);
    }
    js::json config;
    try
    {
        g_log->trace("Read configuration file {}", strFile);
        file >> config;// throws std::exceptions on parse error

        const auto config_end = config.end();

        //-------------- Config loggers ---------------
        auto config_it = config.find("logger");
        if (config_it != config_end)
        {
            const auto end = config_it->end();

            // redirect loggers, if requested
            auto it = config_it->find("log_file");
            if (it != end)
            {
                const auto filename = it->get<std::string>();
                if (filename.empty())
                    g_log->warn("\"log_file\" is empty. logging to console.");
                else
                {
                    const std::string name = g_log->name();
                    spdlog::drop(name);
                    g_log.reset();
                    g_log = spdlog::basic_logger_mt(name, filename);
                }
            }
            it = config_it->find("trace_file");
            if (it != end)
            {
                const auto filename = it->get<std::string>();
                if (filename.empty())
                    g_log->warn("\"trace_file\" is empty. trace-logging to console.");
                else
                {
                    const std::string name = g_logTrace->name();
                    spdlog::drop(name);
                    g_logTrace.reset();
                    g_logTrace = spdlog::basic_logger_mt(name, filename);
                }
            }

            // configure loggers
            it = config_it->find("level");
            if (it != end)
                g_log->set_level(static_cast<spdlog::level::level_enum>(it->get<unsigned>()));
            else
                g_log->warn("Could not set logging level. Default to {}", g_log->level());

            it = config_it->find("trace_calls");
            if (it != end)
                g_logTrace->set_level((it->get<bool>()) ? spdlog::level::trace : spdlog::level::off);

        

        }
        else
            g_log->warn("Loggers are not configured. Using Defaults.");

        g_log->trace("Call Log level: {} ({})", g_logTrace->level(), spdlog::level::to_str(g_logTrace->level()));
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

                // traceing
                const auto trace_it = dev.find("trace");
                std::shared_ptr<spdlog::logger> tracer = nullptr;
                if (trace_it != dev.end())
                if (trace_it.value().get<bool>())
                {
                    std::string trace_file = "";
                    const auto trace_file_it = dev.find("trace_file");
                    if (trace_file_it != dev.end())
                        trace_file = trace_file_it.value().get<std::string>();

                    if (trace_file.empty())
                        tracer = g_logTrace;
                    else
                        tracer = spdlog::basic_logger_mt(dev_name, trace_file);
                }

                

                // streaming files
                const auto skeleton_file_it = dev.find("skeleton_file");
                if (skeleton_file_it == dev.end())
                {
                    g_log->critical("Could not find skeleton_file in configuration. Can not create Fake Device \"{}\"", dev_name);
                    continue;
                }
                const auto skeleton_file = skeleton_file_it.value().get<std::string>();
                if (!file_exists(skeleton_file.c_str()))
                    g_log->warn("File given by \"skeleton_file\" does not exist.\n skeleton_file : {}", skeleton_file);

                const auto color_file_it = dev.find("color_file");
                std::string color_file;
                if (color_file_it == dev.end())
                    g_log->warn("Could not find color_file in configuration. Color Stream will be not avaiable on Device \"{}\"", dev_name);
                else
                    color_file = color_file_it.value().get<std::string>();
                if (!file_exists(skeleton_file.c_str()))
                    g_log->warn("File given by \"color_file\" does not exists.\n color_file: {}", color_file);


                // device creation
                g_devices.push_back( //throws out_of_memory
                    std::make_unique<FakeDevice>(
                        FakeDevice{
                    dev_name,
                    skeleton_file, //skeleton file
                    color_file,
                    tracer, 
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
        return std::make_error_code(std::errc::protocol_error);
    }
    catch (...)
    {
        g_log->critical("Kinect Faker: Unknown Error while reading config file.");
        return std::make_error_code(std::errc::io_error);
    }

    return std::error_code();
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
        
        g_logTrace = spdlog::stdout_logger_mt("call");
        g_logTrace->set_level(spdlog::level::off);
        
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
        is_proxy_init = !create_devices();

    }
        break;
    case DLL_PROCESS_DETACH:
        //cleanup
        if (kinectHndl) FreeLibrary(kinectHndl);
        kinectHndl = NULL;
        is_proxy_init = false;
        g_devices.clear();
        if (g_singleDevice) { g_singleDevice->Release(); g_singleDevice = nullptr; }
        spdlog::drop_all();

        break;
    }
    return is_proxy_init || kinectHndl;

}

template<typename R, typename ...Args>
outcome::result<R> call_nui(const char* name, Args&&... a)
{
    if (!kinectHndl)
        return std::errc::host_unreachable;

    static std::unordered_map<const char*, FARPROC> cached_procs;

    g_logTrace->trace(name);
    typedef R(*funcT)(std::remove_reference_t<Args>...);
    auto it = cached_procs.find(name);
    if (it == cached_procs.end())
        it = cached_procs.emplace(name, GetProcAddress(kinectHndl, name)).first;
    funcT f = reinterpret_cast<funcT>(it->second);
    //enable /std:c++17
    //if constexpr( std::is_same<R,void>::value)
    //{
    //    reinterpret_cast<func>(it->second)(std::forward<Args...>(a)...);
    //    return outcome::success();
    //}
    //else
    {
        return std::invoke(f, std::forward<Args>(a)...);
    }
}

template<typename R, typename memfuncT, typename... Args>
R call_device(const char* name, memfuncT mem_f, Args&&... a)
{
    if (g_singleDevice && is_proxy_init)
        return std::invoke(mem_f, g_singleDevice, std::forward<Args>(a)...);

    if (const auto r = call_nui<HRESULT>(name, std::forward<Args>(a)...))
    {
        return r.value();
    }
    else
    {
        //else g_singleDvice not init && no original Kinect module -> Not connected    
        return E_NUI_DEVICE_NOT_CONNECTED;
    }
}

// calls given function with HRESULT as return type. For single device mode
#define CALL_DEVICE_H(X, ...) call_device<HRESULT>(#X, &INuiSensor_Faker:: ## X, __VA_ARGS__)

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
    if (index >= static_cast<int>(g_devices.size()))
        return E_NUI_BADINDEX;

    std::ifstream scene_file(g_devices[index]->filename, std::ios::binary);
    if (!scene_file.is_open())
        return E_NUI_BADINDEX;

    INuiSensor_Faker::StreamInfos s;
    s.skeletonFilePath = g_devices[index]->filename;
    s.colorFilePath = g_devices[index]->imageFilename;
    *ppNuiSensor = new (std::nothrow) INuiSensor_Faker(std::move(s), g_devices[index]->connectionId, index + pCount);
    if (*ppNuiSensor && g_devices[index]->tracer)
        *ppNuiSensor = new (std::nothrow) INuiSensor_TracerAdaptor(g_devices[index]->tracer, std::unique_ptr<INuiSensor>(*ppNuiSensor));

    return (*ppNuiSensor) ? S_OK : E_OUTOFMEMORY;
}
HRESULT NUIAPI NuiCameraElevationGetAngle(
    LONG *plAngleDegrees
)
{
    return CALL_DEVICE_H(NuiCameraElevationGetAngle, plAngleDegrees);
}

HRESULT NUIAPI NuiCameraElevationSetAngle(
    LONG lAngleDegrees
)
{
    return CALL_DEVICE_H(NuiCameraElevationSetAngle, lAngleDegrees);
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
    return CALL_DEVICE_H(NuiImageGetColorPixelCoordinatesFromDepthPixel, eColorResolution, pcViewArea, lDepthX, lDepthY, usDepthValue, plColorX, plColorY);
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
    return CALL_DEVICE_H(NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution, eColorResolution, eDepthResolution, pcViewArea, lDepthX, lDepthY, usDepthValue, plColorX, plColorY);
}

HRESULT NUIAPI NuiImageStreamGetNextFrame(
    HANDLE hStream,
    DWORD dwMillisecondsToWait,
    const NUI_IMAGE_FRAME **ppcImageFrame
)
{
    return CALL_DEVICE_H(NuiImageStreamGetNextFrame, hStream, dwMillisecondsToWait, const_cast<NUI_IMAGE_FRAME*>(*ppcImageFrame));
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
    return CALL_DEVICE_H(NuiImageStreamOpen,
        eImageType, eResolution, dwImageFrameFlags, dwFrameLimit, hNextFrameEvent, phStreamHandle);
}

HRESULT NUIAPI NuiImageStreamReleaseFrame(
    HANDLE hStream,
    const NUI_IMAGE_FRAME *pImageFrame
)
{
    return CALL_DEVICE_H(NuiImageStreamReleaseFrame, hStream, const_cast<NUI_IMAGE_FRAME*>(pImageFrame));

}

HRESULT NUIAPI NuiInitialize(
    DWORD dwFlags
)
{
    g_logTrace->trace("{} (dwFlags={})", "NuiInitialize", dwFlags);
    if (g_singleDevice != nullptr)
    {
        g_log->warn("Single Fake Kinect already initialized.");
        return E_NUI_ALREADY_INITIALIZED;
    }
    int sensors;
    NuiGetSensorCount(&sensors);
    return NuiCreateSensorByIndex(sensors-1, reinterpret_cast<INuiSensor**>(&g_singleDevice)); //create from first in kinect list
}

HRESULT NUIAPI NuiSetFrameEndEvent(
    HANDLE hEvent,
    DWORD dwFrameEventFlag
)
{
    return CALL_DEVICE_H(NuiSetFrameEndEvent, hEvent, dwFrameEventFlag);
}

void NUIAPI NuiShutdown()
{
    g_logTrace->trace("{} called", "NuiShutdown");
    if (g_singleDevice)
    {
        g_singleDevice->Release();
        g_singleDevice = nullptr;
    }
}

HRESULT NUIAPI NuiSkeletonGetNextFrame(
    DWORD dwMillisecondsToWait,
    NUI_SKELETON_FRAME *pSkeletonFrame
)
{
    return CALL_DEVICE_H(NuiSkeletonGetNextFrame, dwMillisecondsToWait, pSkeletonFrame);
}

HRESULT NUIAPI NuiSkeletonTrackingDisable()
{
    return CALL_DEVICE_H(NuiSkeletonTrackingDisable);
}

HRESULT NUIAPI NuiSkeletonTrackingEnable(
    HANDLE hNextFrameEvent,
    DWORD dwFlags
)
{
    return CALL_DEVICE_H(NuiSkeletonTrackingEnable, hNextFrameEvent, dwFlags);
}

HRESULT NUIAPI NuiTransformSmooth(
    NUI_SKELETON_FRAME *pSkeletonFrame,
    const NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothingParams
)
{
    return CALL_DEVICE_H(NuiTransformSmooth, pSkeletonFrame, pSmoothingParams);
}

HRESULT NUIAPI NuiCreateSensorById(
    const OLECHAR *strInstanceId,
    INuiSensor **ppNuiSensor
)
{
    if (is_proxy_init)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
        std::string instId = conv1.to_bytes(strInstanceId);
        g_logTrace->trace("{} (strInstanceId={})", "NuiCreateSensorById", instId);
        // search for sensor with the given id
        _bstr_t instance_id = strInstanceId;
        for (size_t i = 0; i < g_devices.size(); ++i)
        {
            const auto d = g_devices[i].get();
            int num_devices;
            NuiGetSensorCount(&num_devices);
            num_devices -= static_cast<int>(g_devices.size());
            if (wcscmp(d->connectionId, instance_id) == 0) //maybe use hashing
            {
                INuiSensor_Faker::StreamInfos s;
                s.skeletonFilePath = d->filename;
                s.colorFilePath = d->imageFilename;
                *ppNuiSensor = new (std::nothrow) INuiSensor_Faker(std::move(s), d->connectionId, num_devices + static_cast<int>(i));
                if (*ppNuiSensor && d->tracer)
                    *ppNuiSensor = new (std::nothrow) INuiSensor_TracerAdaptor(d->tracer, std::unique_ptr<INuiSensor>(*ppNuiSensor));

                return (*ppNuiSensor) ? S_OK: E_OUTOFMEMORY;
            }
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
    return CALL_DEVICE_H(NuiImageStreamSetImageFrameFlags, hStream, dwImageFrameFlags);
}

HRESULT NUIAPI NuiImageStreamGetImageFrameFlags(
    HANDLE hStream,
    DWORD *pdwImageFrameFlags
)
{
    return CALL_DEVICE_H(NuiImageStreamGetImageFrameFlags, hStream, pdwImageFrameFlags);
}

HRESULT NUIAPI NuiGetAudioSource(
    INuiAudioBeam **ppDmo
)
{
    return CALL_DEVICE_H(NuiGetAudioSource, ppDmo);
}

HRESULT NUIAPI NuiSkeletonSetTrackedSkeletons(
    DWORD TrackingIDs[NUI_SKELETON_MAX_TRACKED_COUNT]
)
{
    return CALL_DEVICE_H(NuiSkeletonSetTrackedSkeletons, TrackingIDs);
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
