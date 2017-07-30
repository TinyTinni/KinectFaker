#include <catch.hpp>

#include <Windows.h>
#include <NuiApi.h>

TEST_CASE("init Devices", "[init]")
{

    int n_devices;
    HRESULT hr = NuiGetSensorCount(&n_devices);
    const DWORD init_flags = NUI_INITIALIZE_FLAG_USES_SKELETON;

    REQUIRE(SUCCEEDED(hr));
    REQUIRE(n_devices > 0);

    const auto test_basic_props_fn = [&n_devices, init_flags](INuiSensor* device)
    {
        REQUIRE(device != nullptr);
        REQUIRE(SUCCEEDED(device->NuiStatus()));
        REQUIRE(0 == wcscmp(device->NuiDeviceConnectionId(), L"testingConnection"));
        REQUIRE(device->NuiInstanceIndex() == n_devices - 1);
        REQUIRE(device->NuiInitializationFlags() == init_flags);
    };

    SECTION("create by index")
    {
        INuiSensor* device;
        hr = NuiCreateSensorByIndex(n_devices - 1, &device);
        REQUIRE(SUCCEEDED(hr));
        hr = device->NuiInitialize(init_flags);
        REQUIRE(SUCCEEDED(hr));

        test_basic_props_fn(device);

        device->Release();
    }
    SECTION("create by connection id")
    {
        INuiSensor* device;
        hr = NuiCreateSensorById(L"testingConnection", &device);
        REQUIRE(SUCCEEDED(hr));
        hr = device->NuiInitialize(init_flags);
        REQUIRE(SUCCEEDED(hr));

        test_basic_props_fn(device);
        
        device->Release();
    }
    SECTION("create single device")
    {
        hr = NuiInitialize(init_flags);
        REQUIRE(SUCCEEDED(hr));
        NuiShutdown();
    }
}
