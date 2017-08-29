#include <catch.hpp>

#include <Windows.h>
#include <NuiApi.h>

TEST_CASE("init Devices", "[init]")
{  
    const DWORD init_flags = NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR;

    int n_devices;
    REQUIRE(SUCCEEDED(NuiGetSensorCount(&n_devices)));
    REQUIRE(n_devices > 0);

    const auto test_basic_props_fn = [&n_devices, init_flags](INuiSensor* device)
    {
        REQUIRE(device != nullptr);
        REQUIRE(SUCCEEDED(device->NuiStatus()));
        CHECK(0 == wcscmp(device->NuiDeviceConnectionId(), L"testingConnection"));
        CHECK(device->NuiInstanceIndex() == n_devices - 1);
        CHECK(device->NuiInitializationFlags() == init_flags);
    };

    SECTION("create by index")
    {
        INuiSensor* device;
        REQUIRE(SUCCEEDED(NuiCreateSensorByIndex(n_devices - 1, &device)));
        CHECK(SUCCEEDED(device->NuiInitialize(init_flags)));

        test_basic_props_fn(device);

        device->NuiShutdown();
        device->Release();
    }
    SECTION("create by connection id")
    {
        INuiSensor* device;
        REQUIRE(SUCCEEDED(NuiCreateSensorById(L"testingConnection", &device)));
        CHECK(SUCCEEDED(device->NuiInitialize(init_flags)));

        test_basic_props_fn(device);
        
        device->NuiShutdown();
        device->Release();
    }
    SECTION("create single device")
    {
        REQUIRE(SUCCEEDED(NuiInitialize(init_flags)));
        NuiShutdown();
    }
}

TEST_CASE("get skeleton frame", "[skeleton]")
{
    INuiSensor* device;

    {
        //checked by the init function
        int n_devices;
        NuiGetSensorCount(&n_devices);
        NuiCreateSensorByIndex(n_devices - 1, &device);
        device->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);
    }

    HANDLE nextFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    REQUIRE(SUCCEEDED(device->NuiSkeletonTrackingEnable(nextFrameEvent, 0)));

    //check if the next 10 frames has a tracked skeleton
    for (size_t i = 0; i < 10; ++i)
    {
        CHECK(WaitForSingleObject(nextFrameEvent, 36) == WAIT_OBJECT_0);        NUI_SKELETON_FRAME frame;
        REQUIRE(SUCCEEDED(device->NuiSkeletonGetNextFrame(0, &frame)));
        CHECK(SUCCEEDED(device->NuiTransformSmooth(&frame, NULL)));
        int tracked_skeletons = 0;
        for (size_t j = 0; j < NUI_SKELETON_COUNT; ++j)
        {
            const auto& skd = frame.SkeletonData[j];
            tracked_skeletons += skd.eTrackingState == NUI_SKELETON_TRACKED;
        }

        CHECK(tracked_skeletons == 1);

    }

    device->NuiShutdown();
    device->Release();
}