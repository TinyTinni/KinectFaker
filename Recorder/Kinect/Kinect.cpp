#include "kinect.h"
#include <Windows.h>

#include <NuiSensor.h>
#include <NuiSkeleton.h>
#include <NuiApi.h>

bool Kinect::get_skeleton_position()
{
    if (WAIT_OBJECT_0 != WaitForSingleObject(m_hNextSkeletonEvent, 0))
        false;

    NUI_SKELETON_FRAME skeletonFrame = { 0 };
    HRESULT hr = m_device->NuiSkeletonGetNextFrame(0, &skeletonFrame);
    
    //m_device->NuiTransformSmooth(&skeletonFrame, NULL);
    
    for (int j = 0; j < NUI_SKELETON_COUNT; ++j)
    {
        const NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame.SkeletonData[j].eTrackingState;

        if (NUI_SKELETON_TRACKED == trackingState)
        {
            m_newFrameCb();
            auto& skd = skeletonFrame.SkeletonData[j];
            
            for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
            {
                USHORT depth;
                LONG x, y;
                NuiTransformSkeletonToDepthImage(skd.SkeletonPositions[i], &x, &y, &depth);
                skd.SkeletonPositions[i].x = x;
                skd.SkeletonPositions[i].y = y;
                skd.SkeletonPositions[i].z = depth;
            }
            m_newPointCb(skd);
            return true;
        }
        else if (j + 1 == NUI_SKELETON_COUNT) return false;
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
    iSensorCount = 1;
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