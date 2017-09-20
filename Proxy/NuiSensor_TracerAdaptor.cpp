#include "NuiSensor_TracerAdaptor.h"

#include <utility>

#define TRACE_AND_CALL(f, ...) m_log->trace( #f ); return m_sensor->f (__VA_ARGS__);

INuiSensor_TracerAdaptor::~INuiSensor_TracerAdaptor()
{
}

INuiSensor_TracerAdaptor::INuiSensor_TracerAdaptor(std::shared_ptr<spdlog::logger> logger, std::unique_ptr<INuiSensor> sensor):
    m_log(logger), m_sensor(std::move(sensor))
{
}

ULONG INuiSensor_TracerAdaptor::Release()
{
    return m_sensor->Release();
}

ULONG INuiSensor_TracerAdaptor::AddRef()
{
    return m_sensor->AddRef();
}

HRESULT INuiSensor_TracerAdaptor::QueryInterface(const IID &i, void ** v)
{
    return m_sensor->QueryInterface(i, v);
}

HRESULT INuiSensor_TracerAdaptor::NuiInitialize(DWORD dwFlags)
{
    TRACE_AND_CALL(NuiInitialize, dwFlags);
}

void INuiSensor_TracerAdaptor::NuiShutdown(void)
{
    TRACE_AND_CALL(NuiShutdown);
}

HRESULT INuiSensor_TracerAdaptor::NuiSetFrameEndEvent(HANDLE hEvent, DWORD dwFrameEventFlag)
{
    TRACE_AND_CALL(NuiSetFrameEndEvent, hEvent, dwFrameEventFlag);
}

HRESULT INuiSensor_TracerAdaptor::NuiImageStreamOpen(NUI_IMAGE_TYPE eImageType, NUI_IMAGE_RESOLUTION eResolution, DWORD dwImageFrameFlags, DWORD dwFrameLimit, HANDLE hNextFrameEvent, HANDLE * phStreamHandle)
{
    TRACE_AND_CALL(NuiImageStreamOpen, eImageType, eResolution, dwImageFrameFlags, dwFrameLimit, hNextFrameEvent, phStreamHandle)
}

HRESULT INuiSensor_TracerAdaptor::NuiImageStreamSetImageFrameFlags(HANDLE hStream, DWORD dwImageFrameFlags)
{
    TRACE_AND_CALL(NuiImageStreamSetImageFrameFlags, hStream, dwImageFrameFlags);
}

HRESULT INuiSensor_TracerAdaptor::NuiImageStreamGetImageFrameFlags(HANDLE hStream, DWORD * pdwImageFrameFlags)
{
    TRACE_AND_CALL(NuiImageStreamGetImageFrameFlags, hStream, pdwImageFrameFlags);
}

HRESULT INuiSensor_TracerAdaptor::NuiImageStreamGetNextFrame(HANDLE hStream, DWORD dwMillisecondsToWait, NUI_IMAGE_FRAME * pImageFrame)
{
    TRACE_AND_CALL(NuiImageStreamGetNextFrame, hStream, dwMillisecondsToWait, pImageFrame);
}

HRESULT INuiSensor_TracerAdaptor::NuiImageStreamReleaseFrame(HANDLE hStream, NUI_IMAGE_FRAME * pImageFrame)
{
    TRACE_AND_CALL(NuiImageStreamReleaseFrame, hStream, pImageFrame);
}

HRESULT INuiSensor_TracerAdaptor::NuiImageGetColorPixelCoordinatesFromDepthPixel(NUI_IMAGE_RESOLUTION eColorResolution, const NUI_IMAGE_VIEW_AREA * pcViewArea, LONG lDepthX, LONG lDepthY, USHORT usDepthValue, LONG * plColorX, LONG * plColorY)
{
    TRACE_AND_CALL(NuiImageGetColorPixelCoordinatesFromDepthPixel, eColorResolution, pcViewArea, lDepthX, lDepthY, usDepthValue, plColorX, plColorY);
}

HRESULT INuiSensor_TracerAdaptor::NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(NUI_IMAGE_RESOLUTION eColorResolution, NUI_IMAGE_RESOLUTION eDepthResolution, const NUI_IMAGE_VIEW_AREA * pcViewArea, LONG lDepthX, LONG lDepthY, USHORT usDepthValue, LONG * plColorX, LONG * plColorY)
{
    TRACE_AND_CALL(NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution, eColorResolution, eDepthResolution, pcViewArea, lDepthX, lDepthY, usDepthValue, plColorX, plColorY);
}

HRESULT INuiSensor_TracerAdaptor::NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution(NUI_IMAGE_RESOLUTION eColorResolution, NUI_IMAGE_RESOLUTION eDepthResolution, DWORD cDepthValues, USHORT * pDepthValues, DWORD cColorCoordinates, LONG * pColorCoordinates)
{
    TRACE_AND_CALL(NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution, eColorResolution, eDepthResolution, cDepthValues, pDepthValues, cColorCoordinates,pColorCoordinates);

}

HRESULT INuiSensor_TracerAdaptor::NuiCameraElevationSetAngle(LONG lAngleDegrees)
{
    TRACE_AND_CALL(NuiCameraElevationSetAngle, lAngleDegrees);
}

HRESULT INuiSensor_TracerAdaptor::NuiCameraElevationGetAngle(LONG * plAngleDegrees)
{
    TRACE_AND_CALL(NuiCameraElevationGetAngle, plAngleDegrees);
}

HRESULT INuiSensor_TracerAdaptor::NuiSkeletonTrackingEnable(HANDLE hNextFrameEvent, DWORD dwFlags)
{
    TRACE_AND_CALL(NuiSkeletonTrackingEnable, hNextFrameEvent, dwFlags);
}

HRESULT INuiSensor_TracerAdaptor::NuiSkeletonTrackingDisable(void)
{
    TRACE_AND_CALL(NuiSkeletonTrackingDisable);
}

HRESULT INuiSensor_TracerAdaptor::NuiSkeletonSetTrackedSkeletons(DWORD * TrackingIDs)
{
    TRACE_AND_CALL(NuiSkeletonSetTrackedSkeletons, TrackingIDs);
}

HRESULT INuiSensor_TracerAdaptor::NuiSkeletonGetNextFrame(DWORD dwMillisecondsToWait, NUI_SKELETON_FRAME * pSkeletonFrame)
{
    TRACE_AND_CALL(NuiSkeletonGetNextFrame, dwMillisecondsToWait, pSkeletonFrame);
}

HRESULT INuiSensor_TracerAdaptor::NuiTransformSmooth(NUI_SKELETON_FRAME * pSkeletonFrame, const NUI_TRANSFORM_SMOOTH_PARAMETERS * pSmoothingParams)
{
    TRACE_AND_CALL(NuiTransformSmooth, pSkeletonFrame, pSmoothingParams);
}

HRESULT INuiSensor_TracerAdaptor::NuiGetAudioSource(INuiAudioBeam ** ppDmo)
{
    TRACE_AND_CALL(NuiGetAudioSource, ppDmo);
}

int INuiSensor_TracerAdaptor::NuiInstanceIndex(void)
{
    TRACE_AND_CALL(NuiInstanceIndex);
}

BSTR INuiSensor_TracerAdaptor::NuiDeviceConnectionId(void)
{
    TRACE_AND_CALL(NuiDeviceConnectionId);
}

BSTR INuiSensor_TracerAdaptor::NuiUniqueId(void)
{
    TRACE_AND_CALL(NuiUniqueId);
}

BSTR INuiSensor_TracerAdaptor::NuiAudioArrayId(void)
{
    TRACE_AND_CALL(NuiAudioArrayId);
}

HRESULT INuiSensor_TracerAdaptor::NuiStatus(void)
{
    TRACE_AND_CALL(NuiStatus);
}

DWORD INuiSensor_TracerAdaptor::NuiInitializationFlags(void)
{
    TRACE_AND_CALL(NuiInitializationFlags);
}

HRESULT INuiSensor_TracerAdaptor::NuiGetCoordinateMapper(INuiCoordinateMapper ** pMapping)
{
    TRACE_AND_CALL(NuiGetCoordinateMapper, pMapping);
}

HRESULT INuiSensor_TracerAdaptor::NuiImageFrameGetDepthImagePixelFrameTexture(HANDLE hStream, NUI_IMAGE_FRAME * pImageFrame, BOOL * pNearMode, INuiFrameTexture ** ppFrameTexture)
{
    TRACE_AND_CALL(NuiImageFrameGetDepthImagePixelFrameTexture, hStream, pImageFrame, pNearMode, ppFrameTexture);
}

HRESULT INuiSensor_TracerAdaptor::NuiGetColorCameraSettings(INuiColorCameraSettings ** pCameraSettings)
{
    TRACE_AND_CALL(NuiGetColorCameraSettings, pCameraSettings);
}

BOOL INuiSensor_TracerAdaptor::NuiGetForceInfraredEmitterOff(void)
{
    TRACE_AND_CALL(NuiGetForceInfraredEmitterOff);
}

HRESULT INuiSensor_TracerAdaptor::NuiSetForceInfraredEmitterOff(BOOL fForceInfraredEmitterOff)
{
    TRACE_AND_CALL(NuiSetForceInfraredEmitterOff, fForceInfraredEmitterOff);
}

HRESULT INuiSensor_TracerAdaptor::NuiAccelerometerGetCurrentReading(Vector4 * pReading)
{
    TRACE_AND_CALL(NuiAccelerometerGetCurrentReading, pReading);
}

HRESULT INuiSensor_TracerAdaptor::NuiSetDepthFilter(INuiDepthFilter * pDepthFilter)
{
    TRACE_AND_CALL(NuiSetDepthFilter, pDepthFilter);
}

HRESULT INuiSensor_TracerAdaptor::NuiGetDepthFilter(INuiDepthFilter ** ppDepthFilter)
{
    TRACE_AND_CALL(NuiGetDepthFilter, ppDepthFilter);
}

HRESULT INuiSensor_TracerAdaptor::NuiGetDepthFilterForTimeStamp(LARGE_INTEGER liTimeStamp, INuiDepthFilter ** ppDepthFilter)
{
    TRACE_AND_CALL(NuiGetDepthFilterForTimeStamp, liTimeStamp, ppDepthFilter);
}
