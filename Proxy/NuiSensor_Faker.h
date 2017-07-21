#pragma once

#include <Windows.h>
#include <NuiApi.h>
#include <KinectFileDef.pb.h>
#include <comutil.h> // _bstr_t

class INuiSensor_Faker : public INuiSensor
    {
    // COM data
    ULONG m_cRef;
    const _bstr_t m_connectionId;
    const int m_connectionIndex;
    DWORD m_initFlags;
    
    // skeleton data
    kif::Scene m_scene;
    int m_currentFrameIdx;


    HANDLE m_nextFrameTimer;
    // event handles
    HANDLE m_nextSkeletonEvent;

    public:
        ~INuiSensor_Faker();
        INuiSensor_Faker(kif::Scene s, _bstr_t connectionId, int index);


        // INuiSensor overloaded functions
        virtual ULONG Release();
        virtual ULONG AddRef();
        virtual HRESULT QueryInterface(const IID&, void**);


        virtual HRESULT STDMETHODCALLTYPE NuiInitialize( 
            /* [in] */ DWORD dwFlags);
        
        virtual void STDMETHODCALLTYPE NuiShutdown( void);
        
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
        
        virtual HRESULT STDMETHODCALLTYPE NuiSkeletonTrackingDisable( void);
        
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
        
        virtual int STDMETHODCALLTYPE NuiInstanceIndex( void);
        
        virtual BSTR STDMETHODCALLTYPE NuiDeviceConnectionId( void);
        
        virtual BSTR STDMETHODCALLTYPE NuiUniqueId( void);
        
        virtual BSTR STDMETHODCALLTYPE NuiAudioArrayId( void);
        
        virtual HRESULT STDMETHODCALLTYPE NuiStatus( void);
        
        virtual DWORD STDMETHODCALLTYPE NuiInitializationFlags( void);
        
        virtual HRESULT STDMETHODCALLTYPE NuiGetCoordinateMapper( 
            /* [retval][out] */ INuiCoordinateMapper **pMapping);
        
        virtual HRESULT STDMETHODCALLTYPE NuiImageFrameGetDepthImagePixelFrameTexture( 
            /* [in] */ HANDLE hStream,
            /* [in] */ NUI_IMAGE_FRAME *pImageFrame,
            /* [out] */ BOOL *pNearMode,
            /* [out] */ INuiFrameTexture **ppFrameTexture);
        
        virtual HRESULT STDMETHODCALLTYPE NuiGetColorCameraSettings( 
            /* [retval][out] */ INuiColorCameraSettings **pCameraSettings);
        
        virtual BOOL STDMETHODCALLTYPE NuiGetForceInfraredEmitterOff( void);
        
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