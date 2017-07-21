#pragma once

#include <Windows.h>
#include <NuiApi.h>
#include <array>

constexpr int skeleton_joints = NUI_SKELETON_POSITION_COUNT;

class Kinect
{
    INuiSensor* m_device;
    HANDLE m_hNextSkeletonEvent;
    bool m_isOn;

    void transformPoint(float* out, const Vector4& skd);

public:
    Kinect();
    /// get skeleton joint positions in range of [0,1]
    bool get_skeleton_position(float* out);
    bool isOn() { return m_isOn; }
    void enable();
    ~Kinect();
};