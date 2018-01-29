#include "NuiSensor_Faker.h"

#include <fstream>
#include <utility>
#include <new>
#include <system_error>

#include <KinectFileDef.pb.h>

VOID CALLBACK FrameCb(
    _In_ PVOID   lpParameter,
    _In_ BOOLEAN TimerOrWaitFired
)
{
    UNREFERENCED_PARAMETER(TimerOrWaitFired);
    HANDLE* pEvent = reinterpret_cast<HANDLE*>(lpParameter);
    SetEvent(*pEvent);
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

INuiSensor_Faker::~INuiSensor_Faker()
{
    if (m_nextSkeletonFrameTimer != NULL) DeleteTimerQueueTimer(0,m_nextSkeletonFrameTimer,0);
}

INuiSensor_Faker::INuiSensor_Faker(StreamInfos s, _bstr_t connectionId, int index) :
    m_connectionId(std::move(connectionId)),
    m_connectionIndex(std::move(index)),
    m_scene(),
    m_streamPaths(std::move(s))
{
}

HRESULT INuiSensor_Faker::NuiInitialize(DWORD dwFlags)
{
    HRESULT hr = S_OK;
    m_initFlags = 0;
    try {
        if (dwFlags & NUI_INITIALIZE_FLAG_USES_SKELETON)
        {
            std::ifstream scene_file(m_streamPaths.skeletonFilePath, std::ios::binary);
            if (scene_file.is_open())
                if (!m_scene.ParseFromIstream(&scene_file))
                    hr |= E_NUI_FEATURE_NOT_INITIALIZED;
                else
                    m_initFlags |= NUI_INITIALIZE_FLAG_USES_SKELETON;
        }

        if (dwFlags & NUI_INITIALIZE_FLAG_USES_COLOR)
        {
            try 
            {
                m_videoStream = std::make_unique<Video>(m_streamPaths.colorFilePath.c_str());
                m_imageCached = std::make_unique<INuiFrameTexture_Faker>(*(m_videoStream->currentFrame));
            }
            catch (std::runtime_error&)
            {
                hr |= E_NUI_FEATURE_NOT_INITIALIZED;
            }
            m_initFlags |= NUI_INITIALIZE_FLAG_USES_COLOR;
        }
    }
    catch (std::bad_alloc& )
    {
        NuiShutdown();
        return E_OUTOFMEMORY;
    }

    return hr;
}

void INuiSensor_Faker::NuiShutdown(void)
{
    CloseHandle(m_nextSkeletonEvent);
    if (m_nextSkeletonFrameTimer != NULL) DeleteTimerQueueTimer(0, m_nextSkeletonFrameTimer, 0);
    m_nextSkeletonFrameTimer = NULL;
    m_initFlags = 0;

    m_scene.Clear();
    m_videoStream.reset();
    m_imageCached.reset();
}

HRESULT INuiSensor_Faker::NuiSetFrameEndEvent(HANDLE hEvent, DWORD dwFrameEventFlag)
{
    return E_NOTIMPL;
}

HRESULT INuiSensor_Faker::NuiImageStreamOpen(NUI_IMAGE_TYPE eImageType, NUI_IMAGE_RESOLUTION eResolution, DWORD dwImageFrameFlags, DWORD dwFrameLimit, HANDLE hNextFrameEvent, HANDLE * phStreamHandle)
{
    m_nextImageEvent = hNextFrameEvent;
    const bool r = CreateTimerQueueTimer(
        phStreamHandle,
        NULL, //TimerQueue
        FrameCb,
        &m_nextImageEvent,
        30,
        30,//todo: save fps in file?
        WT_EXECUTEDEFAULT
    );
    return S_OK;
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
    if (hStream == NULL) E_INVALIDARG;
    if (m_nextImageEvent == NULL) E_POINTER;

    AVFrame* n = m_videoStream->nextFrame();
    if (!n)
    {
        m_videoStream->restart();
        n = m_videoStream->nextFrame();
        if (!n)
            return E_NUI_FRAME_NO_DATA;
    }
    m_imageCached->setNewFrame(*n);

    pImageFrame->pFrameTexture = m_imageCached.get();
    ResetEvent(m_nextImageEvent);
    return S_OK;
}

HRESULT INuiSensor_Faker::NuiImageStreamReleaseFrame(HANDLE hStream, NUI_IMAGE_FRAME * pImageFrame)
{
    return S_OK;
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
    if (m_scene.frames_size() == 0) return ERROR_INVALID_OPERATION;

    if (m_scene.frames_size() == 0) return ERROR_INVALID_OPERATION;

    m_nextSkeletonEvent = hNextFrameEvent;
    const bool r = CreateTimerQueueTimer(
        &m_nextSkeletonFrameTimer,
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
    DeleteTimerQueueTimer(0, m_nextSkeletonFrameTimer, 0);
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
    //return (m_scene.frames_size() > 0) ? S_OK : E_NUI_NOTCONNECTED;
    return S_OK;
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

INuiSensor_Faker::Video::Video(const char * filename)
{
    av_register_all();
#ifdef _DEBUG
    av_log_set_level(AV_LOG_DEBUG);
#endif
    av_error_code averror;

    averror = avformat_open_input(&formatCtx, filename, nullptr, nullptr);
    if (!averror)
        throw av_error(averror, filename);

    averror = avformat_find_stream_info(formatCtx, nullptr);
    if (!averror)
        throw av_error(averror, filename);

    streamIdx = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (streamIdx < 0)
        throw av_error(streamIdx, filename);

    stream = formatCtx->streams[streamIdx];
   
    codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec)
        throw std::bad_alloc();

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx)
        throw std::bad_alloc(); 

    codecCtx->refcounted_frames = 0;
    avcodec_parameters_to_context(codecCtx, stream->codecpar);
    avcodec_open2(codecCtx, codec, NULL);

    av_init_packet(&pkg);

    videoFrame = av_frame_alloc();
    if (!videoFrame)
        throw std::bad_alloc();
    videoFrame->width = codecCtx->width;
    videoFrame->height = codecCtx->height;
    videoFrame->format = codecCtx->pix_fmt;
    av_image_alloc(videoFrame->data, videoFrame->linesize, codecCtx->width, codecCtx->height, codecCtx->pix_fmt, 0);
    
    currentFrame = av_frame_alloc();
    if (!currentFrame)
        throw std::bad_alloc();
    currentFrame->width = nui_width;
    currentFrame->height = nui_height;
    currentFrame->format = nui_pix_fmt;
    if (0 > av_image_alloc(currentFrame->data, currentFrame->linesize, nui_width, nui_height, nui_pix_fmt, 0))
        throw std::bad_alloc();

    swsCtx = sws_getContext(
        //src
        codecCtx->width,
        codecCtx->height,
        (AVPixelFormat)codecCtx->pix_fmt,
        //dst
        nui_width,
        nui_height,
        (AVPixelFormat)nui_pix_fmt,
        SWS_BICUBIC,
        nullptr, nullptr, nullptr
    );
    if (!swsCtx)
        throw std::bad_alloc();
}

INuiSensor_Faker::Video::~Video()
{
    if (formatCtx) avformat_close_input(&formatCtx);
    if (codecCtx) avcodec_free_context(&codecCtx);
    if (swsCtx) sws_freeContext(swsCtx);
    if (videoFrame) av_frame_free(&videoFrame);
    if (currentFrame) av_frame_free(&currentFrame);
}

AVFrame* INuiSensor_Faker::Video::nextFrame()
{

    while (av_read_frame(formatCtx, &pkg) >= 0)
    {
        if (pkg.stream_index == streamIdx)
        {
            avcodec_send_packet(codecCtx, &pkg);
            avcodec_receive_frame(codecCtx, videoFrame);
            sws_scale(swsCtx, videoFrame->data, videoFrame->linesize, 0, videoFrame->height, currentFrame->data, currentFrame->linesize);
            av_packet_unref(&pkg);
            return currentFrame;
        }
    }
    return nullptr;
}

void INuiSensor_Faker::Video::restart()
{
    av_seek_frame(formatCtx, streamIdx, 0, 0);
    av_packet_free_side_data(&pkg);
}

INuiFrameTexture_Faker::INuiFrameTexture_Faker(AVFrame& f):
    m_cRef(1),
    m_frame(&f)
{
}

INuiFrameTexture_Faker::~INuiFrameTexture_Faker()
{
}

int INuiFrameTexture_Faker::BufferLen(void)
{
    return av_image_get_buffer_size( AVPixelFormat(m_frame->format),
        m_frame->height, m_frame->width, m_frame->linesize[0]);
}

int INuiFrameTexture_Faker::Pitch(void)
{
    return (!m_frame) ? 0 : m_frame->linesize[0];
}

HRESULT INuiFrameTexture_Faker::LockRect(UINT Level, NUI_LOCKED_RECT * pLockedRect, RECT * pRect, DWORD Flags)
{
    if (Level != 0) return E_INVALIDARG;
    if (pLockedRect == nullptr) return E_POINTER;

    pLockedRect->pBits = m_frame->data[0];
    pLockedRect->Pitch = m_frame->linesize[0];
    pLockedRect->size = this->BufferLen();
    return S_OK;
}

HRESULT INuiFrameTexture_Faker::GetLevelDesc(UINT Level, NUI_SURFACE_DESC * pDesc)
{
    if (Level != 0) return E_INVALIDARG;
    if (pDesc == nullptr) return E_POINTER;
    pDesc->Width = m_frame->width;
    pDesc->Height = m_frame->height;
    return S_OK;
}

HRESULT INuiFrameTexture_Faker::UnlockRect(UINT Level)
{
    return S_OK;
}

ULONG INuiFrameTexture_Faker::Release()
{
    // Decrement the object's internal counter.
    ULONG ulRefCount = InterlockedDecrement(&m_cRef);
    if (0 == m_cRef)
    {
        delete this;
    }
    return ulRefCount;
}

ULONG INuiFrameTexture_Faker::AddRef()
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

HRESULT INuiFrameTexture_Faker::QueryInterface(const IID & riid, void ** ppvObj)
{
    // Always set out parameter to NULL, validating it first.
    if (!ppvObj)
        return E_INVALIDARG;
    *ppvObj = NULL;
    if (riid == IID_IUnknown || riid == IID_INuiFrameTexture)
    {
        // Increment the reference count and return the pointer.
        *ppvObj = (LPVOID)this;
        AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}
