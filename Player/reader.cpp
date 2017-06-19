#include "reader.h"
#include <Windows.h>

#include <NuiSensor.h>
#include <NuiSkeleton.h>
#include <NuiApi.h>


bool Kinect::get_skeleton_position(float* output)
{
    if (WAIT_OBJECT_0 != WaitForSingleObject(m_hNextSkeletonEvent, INFINITE))
        false;

    NUI_SKELETON_FRAME skeletonFrame = { 0 };
    HRESULT hr = m_device->NuiSkeletonGetNextFrame(0, &skeletonFrame);
    m_device->NuiTransformSmooth(&skeletonFrame, NULL);


    for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
    {
        const NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame.SkeletonData[i].eTrackingState;

        if (NUI_SKELETON_TRACKED == trackingState)
        {
            // We're tracking the skeleton, draw it
            const auto& skd = skeletonFrame.SkeletonData[i];
            for (i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
                transformPoint(&output[i * 2 * 3], skd.SkeletonPositions[i]);

            //todo: fill
            auto cpy = [&output](const int srcIdx, const int dstIdx)
            {
                memcpy(&output[3 * (2 * dstIdx + 1)], &output[3 * 2 * srcIdx], 3 * sizeof(float));
            };

            cpy(NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_HEAD); //fill otherwise empty slot
            cpy(NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER);
            cpy(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT);
            cpy(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT);
            cpy(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE);
            cpy(NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER);
            cpy(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT);
            cpy(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT);

            // Left Arm
            cpy(NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT);
            cpy(NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT);
            cpy(NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);

            // Right Arm
            cpy(NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT);
            cpy(NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT);
            cpy(NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);

            // Left Leg
            cpy(NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT);
            cpy(NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT);
            cpy(NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);

            // Right Leg
            cpy(NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT);
            cpy(NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT);
            cpy(NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);
        }
        else if (i == NUI_SKELETON_COUNT - 1) return false;
    }

    return true;
}

Kinect::~Kinect()
{
    if (m_device)
    {
        m_device->NuiShutdown();
        m_device->Release();
    }
    if (m_hNextSkeletonEvent != INVALID_HANDLE_VALUE)
        CloseHandle(m_hNextSkeletonEvent);
}

void Kinect::transformPoint(float* out, const Vector4& skd)
{
    // Kinect resolution
    constexpr int        cScreenWidth = 320;
    constexpr int        cScreenHeight = 240;

    LONG x, y;
    USHORT depth;

    // Calculate the skeleton's position on the screen
    // NuiTransformSkeletonToDepthImage returns coordinates in NUI_IMAGE_RESOLUTION_320x240 space
    NuiTransformSkeletonToDepthImage(skd, &x, &y, &depth);

    float a  =out[0] = 0.5f*(-1.f + 2.f*static_cast<float>(x)*1.f / cScreenWidth);
    float b =out[1] = 0.5f*(1.f - 2.f*static_cast<float>(y)*1.f / cScreenHeight);
    float c=out[2] = 0.f;

}

void Kinect::enable()
{
    INuiSensor * pNuiSensor;

    int iSensorCount = 0;
    HRESULT hr = NuiGetSensorCount(&iSensorCount);
    if (FAILED(hr))
        return;
    //{
    //    return hr;
    //}

    // Look at each Kinect sensor
    for (int i = 0; i < iSensorCount; ++i)
    {
        // Create the sensor so we can check status, if we can't create it, move on to the next
        hr = NuiCreateSensorByIndex(i, &pNuiSensor);
        if (FAILED(hr))
        {
            continue;
        }

        // Get the status of the sensor, and if connected, then we can initialize it
        hr = pNuiSensor->NuiStatus();
        if (S_OK == hr)
        {
            m_device = pNuiSensor;
            break;
        }

        // This sensor wasn't OK, so release it since we're not using it
        pNuiSensor->Release();
    }

    if (!m_device)
        return;

    // Initialize the Kinect and specify that we'll be using skeleton
    hr = m_device->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
    if (SUCCEEDED(hr))
    {
        // Create an event that will be signaled when skeleton data is available
        m_hNextSkeletonEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

        // Open a skeleton stream to receive skeleton data
        hr = m_device->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0);
        m_isOn = true;
    }

}

Kinect::Kinect() :
    m_device(nullptr),
    m_hNextSkeletonEvent(INVALID_HANDLE_VALUE),
    m_isOn(false)
{
    enable();
}