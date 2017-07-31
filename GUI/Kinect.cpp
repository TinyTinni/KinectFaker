#include "kinect.h"
#include <Windows.h>

#include <NuiSensor.h>
#include <NuiSkeleton.h>
#include <NuiApi.h>

#include <utility>

bool RecorderKinect::get_skeleton_position()
{
    if (WAIT_OBJECT_0 != WaitForSingleObject(m_hNextSkeletonEvent, 0))
        false;
    return event_next_frame_fired();    
}

bool RecorderKinect::event_next_frame_fired()
{

    NUI_SKELETON_FRAME skeletonFrame = { 0 };
    HRESULT hr = m_device->NuiSkeletonGetNextFrame(0, &skeletonFrame);

    if (m_smooth_parameters)
        m_device->NuiTransformSmooth(&skeletonFrame, m_smooth_parameters);

    m_newPointCb(skeletonFrame);

    return true;
}

RecorderKinect::~RecorderKinect()
{
    if (m_device)
    {
        //m_device->NuiShutdown();
        //m_device->Release();
    }
    if (m_hNextSkeletonEvent != INVALID_HANDLE_VALUE)
        CloseHandle(m_hNextSkeletonEvent);
}

void RecorderKinect::enable()
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
        // Open a skeleton stream to receive skeleton data
        hr = m_device->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0);
        m_isOn = true;
    }

}

RecorderKinect::RecorderKinect() :
    RecorderKinect(CreateEventW(NULL, TRUE, FALSE, NULL))
{

}
RecorderKinect::RecorderKinect(HANDLE skeletonEventHandle) :
    m_device(nullptr),
    m_hNextSkeletonEvent(skeletonEventHandle),
    m_isOn(false),
    m_smooth_parameters(nullptr) //default, see https://msdn.microsoft.com/en-us/library/nuiskeleton.nui_transform_smooth_parameters.aspx
{
    enable();
}

