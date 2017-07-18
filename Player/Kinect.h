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

    std::function<void()> m_newFrameCb;
    std::function<void(const NUI_SKELETON_DATA& skd)> m_newPointCb;

public:

    // call this, if using internal event
    bool get_skeleton_position();

    bool event_next_frame_fired();

    RecorderKinect();
    RecorderKinect(HANDLE skeletonEventHandle);
    /// get skeleton joint positions in range of [0,1]
    void start_record() { m_device->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent,0); }
    void stop_record() {
        m_device->NuiSkeletonTrackingDisable();
    }

    inline void set_new_frame_callback(std::function<void()> c)
    {
        m_newFrameCb = std::move(c);
    }
    inline void set_new_point_callback(std::function<void(const NUI_SKELETON_DATA& skd)> c)
    {
        m_newPointCb = std::move(c);
    }

    //todo better init
    void enable();
    bool isOn() { return m_isOn; }
    ~RecorderKinect();
};