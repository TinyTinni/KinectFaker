#include "NuiSensor_Faker.h"

#include <fstream>
#include <utility>

#include <KinectFileDef.pb.h>

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

INuiSensor_Faker::~INuiSensor_Faker()
{
    if (m_nextFrameTimer != INVALID_HANDLE_VALUE) DeleteTimerQueueTimer(0,m_nextFrameTimer,0);
}

INuiSensor_Faker::INuiSensor_Faker(StreamInfos s, _bstr_t connectionId, int index) :
    m_cRef(1),
    m_connectionId(std::move(connectionId)),
    m_connectionIndex(std::move(index)),
    m_initFlags(0),
    m_scene(),
    m_currentFrameIdx(0),
    m_nextFrameTimer(INVALID_HANDLE_VALUE),
    m_nextSkeletonEvent(INVALID_HANDLE_VALUE),
    m_streamPaths(std::move(s))
{
}

HRESULT INuiSensor_Faker::NuiInitialize(DWORD dwFlags)
{
    if (!(dwFlags & NUI_INITIALIZE_FLAG_USES_SKELETON))
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    m_initFlags = 0;

    if (dwFlags & NUI_INITIALIZE_FLAG_USES_SKELETON)
    {
        std::ifstream scene_file(m_streamPaths.skeletonFilePath, std::ios::binary);
        if (scene_file.is_open())
            if (!m_scene.ParseFromIstream(&scene_file))
                hr |= E_NUI_HARDWARE_FEATURE_UNAVAILABLE;
            else
                m_initFlags |= NUI_INITIALIZE_FLAG_USES_SKELETON;
    }
    return hr;
}

void INuiSensor_Faker::NuiShutdown(void)
{
    CloseHandle(m_nextSkeletonEvent);
    if (m_nextFrameTimer != INVALID_HANDLE_VALUE) DeleteTimerQueueTimer(0, m_nextFrameTimer, 0);
    m_nextFrameTimer = INVALID_HANDLE_VALUE;
    m_scene.Clear();
    m_initFlags = 0;
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

VOID CALLBACK FrameCb(
    _In_ PVOID   lpParameter,
    _In_ BOOLEAN TimerOrWaitFired
)
{
    UNREFERENCED_PARAMETER(TimerOrWaitFired);
    HANDLE* pEvent = reinterpret_cast<HANDLE*>(lpParameter);
    SetEvent(*pEvent);
}

HRESULT INuiSensor_Faker::NuiSkeletonTrackingEnable(HANDLE hNextFrameEvent, DWORD dwFlags)
{
    if (m_scene.frames_size() == 0) return ERROR_INVALID_OPERATION;

    m_nextSkeletonEvent = hNextFrameEvent;
    const bool r = CreateTimerQueueTimer(
        &m_nextFrameTimer,
        NULL, //TimerQueue
        FrameCb,
        &m_nextSkeletonEvent,
        30, 
        30,//todo: save fps in file?
        WT_EXECUTEDEFAULT
    );
    return (r) ? S_OK : r;
}

HRESULT INuiSensor_Faker::NuiSkeletonTrackingDisable(void)
{
    DeleteTimerQueueTimer(0, m_nextFrameTimer, 0);
    return S_OK;
}

HRESULT INuiSensor_Faker::NuiSkeletonSetTrackedSkeletons(DWORD * TrackingIDs)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiSkeletonGetNextFrame(DWORD dwMillisecondsToWait, NUI_SKELETON_FRAME * pSkeletonFrame)
{
    if (m_scene.frames_size() == 0) return ERROR_INVALID_OPERATION;
    if (!pSkeletonFrame) return E_POINTER;

    const auto VecVecCast = [](const kif::Vector& vec) -> Vector4
    {
        Vector4 rv;
        rv.x = vec.x();
        rv.y = vec.y();
        rv.z = vec.z();
        rv.w = vec.w();
        return rv;
    };

    // NUI_SKELETON_FRAME
    const auto& frame = m_scene.frames(m_currentFrameIdx);
    pSkeletonFrame->liTimeStamp.QuadPart = frame.litimestamp();
    pSkeletonFrame->dwFrameNumber = m_currentFrameIdx;
    pSkeletonFrame->vFloorClipPlane = VecVecCast(frame.vfloorclipplane());
    pSkeletonFrame->vNormalToGravity = VecVecCast(frame.vnormaltogravity());

    // NUI_SKELETON_DATA
    for (int i = 0; i < frame.skeleton_data_size(); ++i)
    {
        NUI_SKELETON_DATA* cur = pSkeletonFrame->SkeletonData;
        const auto& data = frame.skeleton_data(i);
        cur->eTrackingState = (NUI_SKELETON_TRACKING_STATE)data.etrackingstate();
        if (cur->eTrackingState == NUI_SKELETON_NOT_TRACKED)
            continue;
        cur->dwTrackingID = data.dwtrackingid();
        cur->dwEnrollmentIndex = data.dwenrollmentindex();
        cur->dwUserIndex = data.dwuserindex();
        cur->Position = VecVecCast(data.position());
        for (int j = 0; j < data.skeletonpositions_size(); ++j)
        {
            cur->eSkeletonPositionTrackingState[j] = (NUI_SKELETON_POSITION_TRACKING_STATE)data.eskeletonpositiontrackingstate(j);
            cur->SkeletonPositions[j] = VecVecCast(data.skeletonpositions(j));
        }
        cur->dwQualityFlags = data.dwqualityflags();
    }
    const int countFrames = m_scene.frames_size();

    m_currentFrameIdx = (m_currentFrameIdx + 1) % m_scene.frames_size();
    
    ResetEvent(m_nextSkeletonEvent);

    return S_OK;
}

HRESULT INuiSensor_Faker::NuiTransformSmooth(NUI_SKELETON_FRAME * pSkeletonFrame, const NUI_TRANSFORM_SMOOTH_PARAMETERS * pSmoothingParams)
{
    return S_OK;
}

HRESULT INuiSensor_Faker::NuiGetAudioSource(INuiAudioBeam ** ppDmo)
{
    return E_NOTIMPL;
}

int INuiSensor_Faker::NuiInstanceIndex(void)
{
    return m_connectionIndex;
}

BSTR INuiSensor_Faker::NuiDeviceConnectionId(void)
{
    return m_connectionId;
}

BSTR INuiSensor_Faker::NuiUniqueId(void)
{
    return NuiDeviceConnectionId();
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
    return m_initFlags;
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
