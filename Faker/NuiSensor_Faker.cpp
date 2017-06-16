#include "NuiSensor_Faker.h"

#include <iostream>
#include <utility>

void CALLBACK NextFrameProc(
    LPVOID lpArg,               // Data value
    DWORD dwTimerLowValue,      // Timer low value
    DWORD dwTimerHighValue)    // Timer high value

{
    // Formal parameters not used in this example.
    UNREFERENCED_PARAMETER(dwTimerLowValue);
    UNREFERENCED_PARAMETER(dwTimerHighValue);

    HANDLE *pEvents = (HANDLE *)lpArg;
    SetEvent(*pEvents);
}


ULONG INuiSensor_Faker::Release()
{
    // Decrement the object's internal counter.
    ULONG ulRefCount = InterlockedDecrement(&m_cRef);
    if (0 == m_cRef)
    {
        delete this;
    }
    return ulRefCount;
}

ULONG INuiSensor_Faker::AddRef()
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

HRESULT INuiSensor_Faker::QueryInterface(const IID & riid, void ** ppvObj)
{
    // Always set out parameter to NULL, validating it first.
    if (!ppvObj)
        return E_INVALIDARG;
    *ppvObj = NULL;
    if (riid == IID_IUnknown || riid == IID_INuiSensor)
    {
        // Increment the reference count and return the pointer.
        *ppvObj = (LPVOID)this;
        AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}

INuiSensor_Faker::INuiSensor_Faker(kif::Scene s):
    m_cRef(1),
    m_scene(std::move(s)),
    m_currentFrameIdx(0),
    m_nextFrameTimer(),
    m_nextSkeletonEvent()
{
}

HRESULT INuiSensor_Faker::NuiInitialize(DWORD dwFlags)
{
    if (!(dwFlags & NUI_INITIALIZE_FLAG_USES_SKELETON))
        return E_INVALIDARG;

    m_nextFrameTimer = CreateWaitableTimer(
        NULL,                   // Default security attributes
        FALSE,                  // Create auto-reset timer
        NULL);       // Name of waitable timer

    return S_OK;
}

void INuiSensor_Faker::NuiShutdown(void)
{
    CloseHandle(m_nextFrameTimer);
}

HRESULT INuiSensor_Faker::NuiSetFrameEndEvent(HANDLE hEvent, DWORD dwFrameEventFlag)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiImageStreamOpen(NUI_IMAGE_TYPE eImageType, NUI_IMAGE_RESOLUTION eResolution, DWORD dwImageFrameFlags, DWORD dwFrameLimit, HANDLE hNextFrameEvent, HANDLE * phStreamHandle)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiImageStreamSetImageFrameFlags(HANDLE hStream, DWORD dwImageFrameFlags)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiImageStreamGetImageFrameFlags(HANDLE hStream, DWORD * pdwImageFrameFlags)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiImageStreamGetNextFrame(HANDLE hStream, DWORD dwMillisecondsToWait, NUI_IMAGE_FRAME * pImageFrame)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiImageStreamReleaseFrame(HANDLE hStream, NUI_IMAGE_FRAME * pImageFrame)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiImageGetColorPixelCoordinatesFromDepthPixel(NUI_IMAGE_RESOLUTION eColorResolution, const NUI_IMAGE_VIEW_AREA * pcViewArea, LONG lDepthX, LONG lDepthY, USHORT usDepthValue, LONG * plColorX, LONG * plColorY)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(NUI_IMAGE_RESOLUTION eColorResolution, NUI_IMAGE_RESOLUTION eDepthResolution, const NUI_IMAGE_VIEW_AREA * pcViewArea, LONG lDepthX, LONG lDepthY, USHORT usDepthValue, LONG * plColorX, LONG * plColorY)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution(NUI_IMAGE_RESOLUTION eColorResolution, NUI_IMAGE_RESOLUTION eDepthResolution, DWORD cDepthValues, USHORT * pDepthValues, DWORD cColorCoordinates, LONG * pColorCoordinates)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiCameraElevationSetAngle(LONG lAngleDegrees)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiCameraElevationGetAngle(LONG * plAngleDegrees)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiSkeletonTrackingEnable(HANDLE hNextFrameEvent, DWORD dwFlags)
{

    LARGE_INTEGER liDueTime;
    m_nextSkeletonEvent = hNextFrameEvent;
    liDueTime.QuadPart = 1LL;
    return SetWaitableTimer(
        m_nextFrameTimer,           // Handle to the timer object
        &liDueTime,       // When timer will become signaled
        30,             // Periodic timer interval of 2 seconds
        NextFrameProc,     // Completion routine
        &m_nextSkeletonEvent,          // Argument to the completion routine
        FALSE);          // Do not restore a suspended system
}

HRESULT INuiSensor_Faker::NuiSkeletonTrackingDisable(void)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiSkeletonSetTrackedSkeletons(DWORD * TrackingIDs)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiSkeletonGetNextFrame(DWORD dwMillisecondsToWait, NUI_SKELETON_FRAME * pSkeletonFrame)
{
    if (!pSkeletonFrame)
        return E_POINTER;

    const auto& frame = m_scene.frames(m_currentFrameIdx);
    pSkeletonFrame->liTimeStamp;
    pSkeletonFrame->dwFrameNumber = m_currentFrameIdx;
    pSkeletonFrame->vFloorClipPlane = Vector4();
    pSkeletonFrame->vNormalToGravity = Vector4();

    const auto VecVecCast = [](const kif::SkeletonData_Vector& vec) -> Vector4 
    {
        Vector4 rv;
        rv.x = (FLOAT)vec.x();
        rv.y = (FLOAT)vec.y();
        rv.z = (FLOAT)vec.z();
        rv.w = (FLOAT)vec.w();
        return rv;
    };

    for (int i = 0; i < frame.skeleton_data_size(); ++i)
    {
        NUI_SKELETON_DATA* cur = pSkeletonFrame->SkeletonData;
        const auto& data = frame.skeleton_data(i);
        cur->eTrackingState = (NUI_SKELETON_TRACKING_STATE)data.etrackingstate();
        if (cur->eTrackingState == NUI_SKELETON_NOT_TRACKED)
            return S_OK;
        cur->dwTrackingID = data.dwtrackingid();
        cur->dwEnrollmentIndex = data.dwenrollmentindex();
        cur->dwUserIndex = data.dwuserindex();
        cur->Position = VecVecCast(data.position());
        for (int j = 0; j < data.skeletonpositions_size(); ++j)
        {
            //cur->eSkeletonPositionTrackingState[j] = (NUI_SKELETON_POSITION_TRACKING_STATE)data.eskeletonpositiontrackingstate(j);
            cur->SkeletonPositions[j] = VecVecCast(data.skeletonpositions(j));
        }
        cur->dwQualityFlags = data.dwqualityflags();
        /*
        
    NUI_SKELETON_TRACKING_STATE eTrackingState;
    DWORD dwTrackingID;
    DWORD dwEnrollmentIndex;
    DWORD dwUserIndex;
    Vector4 Position;
    Vector4 SkeletonPositions[ 20 ];
    NUI_SKELETON_POSITION_TRACKING_STATE eSkeletonPositionTrackingState[ 20 ];
    DWORD dwQualityFlags;
        */
    }
        /*
        LARGE_INTEGER liTimeStamp;
    DWORD dwFrameNumber;
    DWORD dwFlags;
    Vector4 vFloorClipPlane;
    Vector4 vNormalToGravity;
    NUI_SKELETON_DATA SkeletonData[NUI_SKELETON_COUNT];
        */

    if (++m_currentFrameIdx == m_scene.frames_size())
        m_currentFrameIdx = 0;
    
    Sleep(30);
    ResetEvent(m_nextSkeletonEvent);

    return S_OK;
}

HRESULT INuiSensor_Faker::NuiTransformSmooth(NUI_SKELETON_FRAME * pSkeletonFrame, const NUI_TRANSFORM_SMOOTH_PARAMETERS * pSmoothingParams)
{
    return S_OK;//::NuiTransformSmooth(pSkeletonFrame, pSmoothingParams);
}

HRESULT INuiSensor_Faker::NuiGetAudioSource(INuiAudioBeam ** ppDmo)
{
    return E_NOTIMPL;
}

int INuiSensor_Faker::NuiInstanceIndex(void)
{
    return 0;
}

BSTR INuiSensor_Faker::NuiDeviceConnectionId(void)
{
    return BSTR();
}

BSTR INuiSensor_Faker::NuiUniqueId(void)
{
    return BSTR();
}

BSTR INuiSensor_Faker::NuiAudioArrayId(void)
{
    return BSTR();
}

HRESULT INuiSensor_Faker::NuiStatus(void)
{
    return (m_scene.frames_size()) ? S_OK : E_NUI_NOTCONNECTED;
}

DWORD INuiSensor_Faker::NuiInitializationFlags(void)
{
    return 0;
}

HRESULT INuiSensor_Faker::NuiGetCoordinateMapper(INuiCoordinateMapper ** pMapping)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiImageFrameGetDepthImagePixelFrameTexture(HANDLE hStream, NUI_IMAGE_FRAME * pImageFrame, BOOL * pNearMode, INuiFrameTexture ** ppFrameTexture)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiGetColorCameraSettings(INuiColorCameraSettings ** pCameraSettings)
{
    return E_NOTIMPL;
}

BOOL INuiSensor_Faker::NuiGetForceInfraredEmitterOff(void)
{
    return 0;
}

HRESULT INuiSensor_Faker::NuiSetForceInfraredEmitterOff(BOOL fForceInfraredEmitterOff)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiAccelerometerGetCurrentReading(Vector4 * pReading)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiSetDepthFilter(INuiDepthFilter * pDepthFilter)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiGetDepthFilter(INuiDepthFilter ** ppDepthFilter)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiGetDepthFilterForTimeStamp(LARGE_INTEGER liTimeStamp, INuiDepthFilter ** ppDepthFilter)
{
    return E_NOTIMPL;
}
