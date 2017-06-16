#pragma once

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

    void transformPoint(gsl::span<float, 3>& out, const Vector4& skd);

public:
    Kinect();
    /// get skeleton joint positions in range of [0,1]
    bool get_skeleton_position(gsl::span<float> out);
    bool isOn() { return m_isOn; }
    void enable();
    ~Kinect();
};