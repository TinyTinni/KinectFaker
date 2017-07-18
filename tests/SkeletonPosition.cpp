#include <catch.hpp>

#include <Windows.h>
#include <NuiApi.h>

TEST_CASE("Get Skeleton Positions", "[skeleton]")
{

    int n_devices;
    HRESULT hr = NuiGetSensorCount(&n_devices);

    REQUIRE(SUCCEEDED(hr));
    REQUIRE(n_devices > 0);

    INuiSensor* device;
    hr = NuiCreateSensorByIndex(n_devices - 1, &device);
    REQUIRE(SUCCEEDED(hr));
    REQUIRE(device != nullptr);
    REQUIRE(SUCCEEDED(device->NuiStatus()));



}
