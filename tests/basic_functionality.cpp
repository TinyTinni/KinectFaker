#include <catch.hpp>

#include <Windows.h>
#include <NuiApi.h>

TEST_CASE("init ", "[init]")
{

    int n_devices;
    HRESULT hr = NuiGetSensorCount(&n_devices);

    REQUIRE(SUCCEEDED(hr));
    REQUIRE(n_devices > 0);

    const auto test_basic_props_fn = [](INuiSensor* device, HRESULT hr)
    {
        REQUIRE(SUCCEEDED(hr));
        REQUIRE(device != nullptr);
        REQUIRE(SUCCEEDED(device->NuiStatus()));
        REQUIRE(0 == wcscmp(device->NuiDeviceConnectionId(), L"testingConnection"));
    };

    SECTION("create by index")
    {
        INuiSensor* device;
        hr = NuiCreateSensorByIndex(n_devices - 1, &device);

        test_basic_props_fn(device, hr);

        device->Release();
    }
    SECTION("create by connection id")
    {
        INuiSensor* device;
        hr = NuiCreateSensorById(L"testingConnection", &device);

        test_basic_props_fn(device, hr);
        
        device->Release();
    }
}
