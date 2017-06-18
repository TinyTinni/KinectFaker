#include "NuiApi_Faker.h"

#include <fstream> //fstream
#include <utility> //move
#include <vector>
#include <memory>

//delete later
#include <iostream>

#include "NuiSensor_Faker.h"
#include "KinectFileDef.pb.h"

#include <string>

std::string fname = "";
int fake_index = -1;

//todo: multi devices
struct FakeDevice
{
    std::string filename;
    size_t idx;
};

std::vector<std::unique_ptr<FakeDevice>> currentDevices;

// use BOOST_PP
//#define HOOKS \
//    (NuiCreateSensorByIndex)\
//    (NuiGetSensorCount)

subhook_t hook_NuiCreateSensorByIndex = nullptr;
subhook_t hook_NuiGetSensorCount = nullptr;


//create global hooks with hook_<functionname> for each function in HOOKS
//#define HOOKS_DEFINE_MACRO(r, data, elem)\
//        subhook_t BOOST_PP_CAT(data, elem);
//BOOST_PP_SEQ_FOR_EACH(HOOKS_DEFINE_MACRO, hook_, HOOKS);


//------------------------ Hook Functions ------------------- //
HRESULT my_NuiCreateSensorByIndex(_In_ int index, _Out_ INuiSensor ** ppNuiSensor)
{
    typedef HRESULT(*f)(int, INuiSensor**);
    int pCount;
    subhook_remove(hook_NuiGetSensorCount);
    NuiGetSensorCount(&pCount);
    subhook_install(hook_NuiGetSensorCount);

    if (index < pCount)
        return ((f)subhook_get_trampoline(hook_NuiCreateSensorByIndex))(index, ppNuiSensor);
    index -= pCount;
    if (index >= currentDevices.size())
        return ERROR_INVALID_INDEX; //msdn doc is wrong with returning values. Check correct value here

    std::ifstream scene_file(currentDevices[index]->filename, std::ios::binary);
    if (!scene_file.is_open())
        return E_OUTOFMEMORY;

    kif::Scene scene;
    if (!scene.ParseFromIstream(&scene_file))
        return E_OUTOFMEMORY;
    
    auto frames = scene.frames_size();
    *ppNuiSensor = new INuiSensor_Faker(std::move(scene));
    return S_OK;
}


HRESULT my_NuiGetSensorCount(_In_ int * pCount)
{
    typedef HRESULT(*f)(int*);
    //HRESULT r = ((f)subhook_get_trampoline(hook_NuiGetSensorCount))(pCount); // crashes... why? idk
    subhook_remove(hook_NuiGetSensorCount);
    HRESULT r = NuiGetSensorCount(pCount);
    subhook_install(hook_NuiGetSensorCount);


    if (FAILED(r))
        return r;
    *pCount += currentDevices.size();
    return S_OK;
}


//------------------------ Lib Functions ------------------- //
fake_kinect_t fake_kinect_add(const char* strFile)
{
    if (!hook_NuiGetSensorCount)
    {
        hook_NuiGetSensorCount = subhook_new((void*)NuiGetSensorCount, (void*)my_NuiGetSensorCount, SUBHOOK_OPTION_64BIT_OFFSET);
        hook_NuiCreateSensorByIndex = subhook_new((void*)NuiCreateSensorByIndex, (void*)my_NuiCreateSensorByIndex, SUBHOOK_OPTION_64BIT_OFFSET);
        // install hooks
        subhook_install(hook_NuiCreateSensorByIndex);
        subhook_install(hook_NuiGetSensorCount);
    }

    // check, if file exists (change with c++17 filesystem in the future
    {
        std::ifstream scene_file;
        scene_file.open(strFile, std::ios::in);
        if (!scene_file.is_open())
            return nullptr;
    }

    try
    {
        currentDevices.push_back(
            std::make_unique<FakeDevice>(
                FakeDevice{strFile, currentDevices.size()}
                )
        );
    }
    catch (...)
    {
        return nullptr;
    }
    fake_kinect_t ret = currentDevices.back().get();

    return ret;

}
void fake_kinect_free(fake_kinect_t& hnd)
{
    if (!hnd)
        return;

    const auto it = std::find_if(std::cbegin(currentDevices), std::cend(currentDevices),
        [hnd](const auto& uptr) {return uptr.get() == hnd; });
    if (it != std::cend(currentDevices))
        currentDevices.erase(it);
    hnd = nullptr;

    if (currentDevices.empty() && hook_NuiCreateSensorByIndex)
    {
        subhook_remove(hook_NuiCreateSensorByIndex);
        subhook_remove(hook_NuiGetSensorCount);

        subhook_free(hook_NuiCreateSensorByIndex);
        subhook_free(hook_NuiGetSensorCount);
    }
}


fake_kinect_t g_singlesupport = nullptr;

HRESULT kinect_faker_init(const char* strFile)
{
    if (g_singlesupport)
        kinect_faker_release();
    g_singlesupport = fake_kinect_add(strFile);
    return (g_singlesupport)?S_OK: STG_E_FILENOTFOUND;
}
void kinect_faker_release()
{
    if (g_singlesupport)
        fake_kinect_free(g_singlesupport);
}