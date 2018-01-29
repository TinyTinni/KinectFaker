#pragma once

#include <Windows.h>
#include <NuiApi.h>
#include <KinectFileDef.pb.h>
#include <comutil.h> // _bstr_t

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

#include "av_error.hpp"

class INuiFrameTexture_Faker: public INuiFrameTexture
{

private:
    struct AVFrameDeleter
    {
        void operator()(AVFrame* f)
        {
            if (f) av_frame_free(&f);
        }
    };

public:
    using unique_avframe_ptr = std::unique_ptr<AVFrame, AVFrameDeleter>;

    ULONG m_cRef;
    AVFrame* m_frame;
public:
    void setNewFrame(AVFrame& n) { m_frame = &n; }
    INuiFrameTexture_Faker(AVFrame& f);
    ~INuiFrameTexture_Faker();

    virtual int STDMETHODCALLTYPE BufferLen(void);

    virtual int STDMETHODCALLTYPE Pitch(void);

    virtual HRESULT STDMETHODCALLTYPE LockRect(
        UINT Level,
        /* [ref] */ NUI_LOCKED_RECT *pLockedRect,
        /* [unique] */ RECT *pRect,
        DWORD Flags);

    virtual HRESULT STDMETHODCALLTYPE GetLevelDesc(
        UINT Level,
        NUI_SURFACE_DESC *pDesc);

    virtual HRESULT STDMETHODCALLTYPE UnlockRect(
        /* [in] */ UINT Level);

    virtual ULONG STDMETHODCALLTYPE Release();
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID&, void**);

};


class INuiSensor_Faker : public INuiSensor
{
public:
    struct StreamInfos
    {
        std::string skeletonFilePath;
        std::string colorFilePath;
        std::string depthFilePath;
    };
private:

    struct Video
    {
    public:
        using unique_avframe_ptr = INuiFrameTexture_Faker::unique_avframe_ptr;
        AVFormatContext* formatCtx = nullptr;
        AVStream* stream = nullptr;
        int streamIdx = -1;
        AVCodec* codec = nullptr;
        AVCodecContext* codecCtx = nullptr;
        SwsContext* swsCtx = nullptr;
        AVPacket pkg;

        AVFrame* videoFrame = nullptr; // non transformed frame from the video
        AVFrame* currentFrame = nullptr; // transformed frame to NUI formatA

        static const auto nui_pix_fmt = AV_PIX_FMT_RGB32;
        static const int nui_width = 640;
        static const int nui_height = 480;

        Video(const char* filename);
        ~Video();

        // returns the next frame. This frame is just borrowed and will be invalid on the next "nextFrame" call of destroying
        // the Video instance
        // returns nullptr, if eof was reached
        AVFrame* nextFrame();
        void restart();


    };

    // COM data
    ULONG m_cRef = 1;
    const _bstr_t m_connectionId;
    const int m_connectionIndex;
    DWORD m_initFlags = -1;

    // skeleton data
    kif::Scene m_scene;
    int m_currentFrameIdx = 0;

    // image stream data
    std::unique_ptr<Video> m_videoStream = nullptr;
    std::unique_ptr<INuiFrameTexture_Faker> m_imageCached = nullptr;

    // timers
    HANDLE m_nextSkeletonFrameTimer = NULL;
    HANDLE m_nextImageFrameTimer = NULL;
    // event handles
    HANDLE m_nextSkeletonEvent = NULL;
    HANDLE m_nextImageEvent = NULL;
    const StreamInfos m_streamPaths;


public:

    INuiSensor_Faker(StreamInfos s, _bstr_t connectionId, int index);
    ~INuiSensor_Faker();


    // INuiSensor overloaded functions
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID&, void**);


    virtual HRESULT STDMETHODCALLTYPE NuiInitialize(
        /* [in] */ DWORD dwFlags);

    virtual void STDMETHODCALLTYPE NuiShutdown(void);

    virtual HRESULT STDMETHODCALLTYPE NuiSetFrameEndEvent(
        /* [in] */ HANDLE hEvent,
        /* [in] */ DWORD dwFrameEventFlag);

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamOpen(
        /* [in] */ NUI_IMAGE_TYPE eImageType,
        /* [in] */ NUI_IMAGE_RESOLUTION eResolution,
        /* [in] */ DWORD dwImageFrameFlags,
        /* [in] */ DWORD dwFrameLimit,
        /* [in] */ HANDLE hNextFrameEvent,
        /* [out] */ HANDLE *phStreamHandle);

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamSetImageFrameFlags(
        /* [in] */ HANDLE hStream,
        /* [in] */ DWORD dwImageFrameFlags);

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamGetImageFrameFlags(
        /* [in] */ HANDLE hStream,
        /* [retval][out] */ DWORD *pdwImageFrameFlags);

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamGetNextFrame(
        /* [in] */ HANDLE hStream,
        /* [in] */ DWORD dwMillisecondsToWait,
        /* [retval][out] */ NUI_IMAGE_FRAME *pImageFrame);

    virtual HRESULT STDMETHODCALLTYPE NuiImageStreamReleaseFrame(
        /* [in] */ HANDLE hStream,
        /* [in] */ NUI_IMAGE_FRAME *pImageFrame);

    virtual HRESULT STDMETHODCALLTYPE NuiImageGetColorPixelCoordinatesFromDepthPixel(
        /* [in] */ NUI_IMAGE_RESOLUTION eColorResolution,
        /* [in] */ const NUI_IMAGE_VIEW_AREA *pcViewArea,
        /* [in] */ LONG lDepthX,
        /* [in] */ LONG lDepthY,
        /* [in] */ USHORT usDepthValue,
        /* [out] */ LONG *plColorX,
        /* [out] */ LONG *plColorY);

    virtual HRESULT STDMETHODCALLTYPE NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
        /* [in] */ NUI_IMAGE_RESOLUTION eColorResolution,
        /* [in] */ NUI_IMAGE_RESOLUTION eDepthResolution,
        /* [in] */ const NUI_IMAGE_VIEW_AREA *pcViewArea,
        /* [in] */ LONG lDepthX,
        /* [in] */ LONG lDepthY,
        /* [in] */ USHORT usDepthValue,
        /* [out] */ LONG *plColorX,
        /* [out] */ LONG *plColorY);

    virtual HRESULT STDMETHODCALLTYPE NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution(
        /* [in] */ NUI_IMAGE_RESOLUTION eColorResolution,
        /* [in] */ NUI_IMAGE_RESOLUTION eDepthResolution,
        /* [in] */ DWORD cDepthValues,
        /* [size_is][in] */ USHORT *pDepthValues,
        /* [in] */ DWORD cColorCoordinates,
        /* [size_is][out][in] */ LONG *pColorCoordinates);

    virtual HRESULT STDMETHODCALLTYPE NuiCameraElevationSetAngle(
        /* [in] */ LONG lAngleDegrees);

    virtual HRESULT STDMETHODCALLTYPE NuiCameraElevationGetAngle(
        /* [retval][out] */ LONG *plAngleDegrees);

    virtual HRESULT STDMETHODCALLTYPE NuiSkeletonTrackingEnable(
        /* [in] */ HANDLE hNextFrameEvent,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE NuiSkeletonTrackingDisable(void);

    virtual HRESULT STDMETHODCALLTYPE NuiSkeletonSetTrackedSkeletons(
        /* [size_is][in] */ DWORD *TrackingIDs);

    virtual HRESULT STDMETHODCALLTYPE NuiSkeletonGetNextFrame(
        /* [in] */ DWORD dwMillisecondsToWait,
        /* [out][in] */ NUI_SKELETON_FRAME *pSkeletonFrame);

    virtual HRESULT STDMETHODCALLTYPE NuiTransformSmooth(
        NUI_SKELETON_FRAME *pSkeletonFrame,
        const NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothingParams);

    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NuiGetAudioSource(
        /* [out] */ INuiAudioBeam **ppDmo);

    virtual int STDMETHODCALLTYPE NuiInstanceIndex(void);

    virtual BSTR STDMETHODCALLTYPE NuiDeviceConnectionId(void);

    virtual BSTR STDMETHODCALLTYPE NuiUniqueId(void);

    virtual BSTR STDMETHODCALLTYPE NuiAudioArrayId(void);

    virtual HRESULT STDMETHODCALLTYPE NuiStatus(void);

    virtual DWORD STDMETHODCALLTYPE NuiInitializationFlags(void);

    virtual HRESULT STDMETHODCALLTYPE NuiGetCoordinateMapper(
        /* [retval][out] */ INuiCoordinateMapper **pMapping);

    virtual HRESULT STDMETHODCALLTYPE NuiImageFrameGetDepthImagePixelFrameTexture(
        /* [in] */ HANDLE hStream,
        /* [in] */ NUI_IMAGE_FRAME *pImageFrame,
        /* [out] */ BOOL *pNearMode,
        /* [out] */ INuiFrameTexture **ppFrameTexture);

    virtual HRESULT STDMETHODCALLTYPE NuiGetColorCameraSettings(
        /* [retval][out] */ INuiColorCameraSettings **pCameraSettings);

    virtual BOOL STDMETHODCALLTYPE NuiGetForceInfraredEmitterOff(void);

    virtual HRESULT STDMETHODCALLTYPE NuiSetForceInfraredEmitterOff(
        /* [in] */ BOOL fForceInfraredEmitterOff);

    virtual HRESULT STDMETHODCALLTYPE NuiAccelerometerGetCurrentReading(
        /* [retval][out] */ Vector4 *pReading);

    virtual HRESULT STDMETHODCALLTYPE NuiSetDepthFilter(
        /* [in] */ INuiDepthFilter *pDepthFilter);

    virtual HRESULT STDMETHODCALLTYPE NuiGetDepthFilter(
        /* [retval][out] */ INuiDepthFilter **ppDepthFilter);

    virtual HRESULT STDMETHODCALLTYPE NuiGetDepthFilterForTimeStamp(
        /* [in] */ LARGE_INTEGER liTimeStamp,
        /* [retval][out] */ INuiDepthFilter **ppDepthFilter);

};