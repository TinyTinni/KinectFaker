#pragma once

#include <Windows.h>

#include <NuiApi.h>
#include <array>

#include <functional>
#include <memory>

constexpr int skeleton_joints = NUI_SKELETON_POSITION_COUNT;

class RecorderKinect
{
    INuiSensor* m_device;
    HANDLE m_hNextSkeletonEvent;
    HANDLE m_hNextImageEvent;
    HANDLE m_hImageStream;
    bool m_isOn;

    NUI_IMAGE_FRAME m_frame;
    NUI_TRANSFORM_SMOOTH_PARAMETERS* m_smooth_parameters;
    struct KinectImagePtrDeleter
    {
        KinectImagePtrDeleter(RecorderKinect* t):m_t(t){}
        void operator()(NUI_IMAGE_FRAME* imf)
        {
            m_t->m_device->NuiImageStreamReleaseFrame(m_t->m_hImageStream, imf);
        }
    private:
        RecorderKinect* m_t;
    };

public:
    using UniqueImagePtr = std::unique_ptr<NUI_IMAGE_FRAME, KinectImagePtrDeleter>;

private:

    std::function<void(const NUI_SKELETON_FRAME& skd)> m_newPointCb;
    std::function<void(UniqueImagePtr& f)> m_newImageCb;
public:

    // call this, if using internal event
    bool get_skeleton_position();

    bool event_next_frame_fired();
    bool event_next_color_frame_fired();

    INuiSensor* get_raw_device() { return m_device; }

    void enable_smoothing(NUI_TRANSFORM_SMOOTH_PARAMETERS* p) { m_smooth_parameters = p; }
    void disable_smoothing() { m_smooth_parameters = nullptr; }

    RecorderKinect();
    RecorderKinect(HANDLE skeletonEventHandle, HANDLE imageEventHandle);
    /// get skeleton joint positions in range of [0,1]
    void start_record() { 
        m_device->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent,0);
    }
    void stop_record() {
        m_device->NuiSkeletonTrackingDisable();
    }

    inline void set_new_point_callback(std::function<void(const NUI_SKELETON_FRAME&)> c)
    {
        m_newPointCb = std::move(c);
    }

    inline void set_new_frame_callback(std::function<void(UniqueImagePtr&)> c)
    {
        m_newImageCb = std::move(c);
    }

    bool get_color_frame()
    {
        if (WAIT_OBJECT_0 != WaitForSingleObject(m_hNextImageEvent, 40))
            return false;
        return event_next_color_frame_fired();
    }

    //todo better init
    void enable();
    bool isOn() { return m_isOn; }
    ~RecorderKinect();
};