#pragma once

#define NOMINMAX
#include <Windows.h>
#include <NuiApi.h>
#include <array>

#include <gsl/gsl>

constexpr int skeleton_joints = NUI_SKELETON_POSITION_COUNT;

class Kinect
{
    INuiSensor* m_device;
    HANDLE m_hNextSkeletonEvent;
    bool m_isOn;
    bool m_record;

    std::function<void()> m_newFrameCb;
    std::function<void(int, long, long, unsigned short)> m_newPointCb;

public:

    bool get_skeleton_position();
    Kinect();
    /// get skeleton joint positions in range of [0,1]
    void start_record() { m_record = true; }
    void stop_record() { m_record = false; }

    inline void set_new_frame_callback(std::function<void()> c)
    {
        m_newFrameCb = std::move(c);
    }
    inline void set_new_point_callback(std::function<void(int, long, long, unsigned short)> c)
    {
        m_newPointCb = std::move(c);
    }

    //todo better init
    void enable();
    bool isOn() { return m_isOn; }
    ~Kinect();
};