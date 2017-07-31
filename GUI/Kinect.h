#pragma once

#include <Windows.h>

#include <NuiApi.h>
#include <array>

#include <gsl/gsl>

constexpr int skeleton_joints = NUI_SKELETON_POSITION_COUNT;

class RecorderKinect
{
    INuiSensor* m_device;
    HANDLE m_hNextSkeletonEvent;
    bool m_isOn;

    NUI_TRANSFORM_SMOOTH_PARAMETERS* m_smooth_parameters;

    std::function<void(const NUI_SKELETON_FRAME& skd)> m_newPointCb;

public:

    // call this, if using internal event
    bool get_skeleton_position();

    bool event_next_frame_fired();

    INuiSensor* get_raw_device() { return m_device; }

    void enable_smoothing(NUI_TRANSFORM_SMOOTH_PARAMETERS* p) { m_smooth_parameters = p; }
    void disable_smoothing() { m_smooth_parameters = nullptr; }

    RecorderKinect();
    RecorderKinect(HANDLE skeletonEventHandle);
    /// get skeleton joint positions in range of [0,1]
    void start_record() { m_device->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent,0); }
    void stop_record() {
        m_device->NuiSkeletonTrackingDisable();
    }

    inline void set_new_point_callback(std::function<void(const NUI_SKELETON_FRAME& skd)> c)
    {
        m_newPointCb = std::move(c);
    }

    //todo better init
    void enable();
    bool isOn() { return m_isOn; }
    ~RecorderKinect();
};