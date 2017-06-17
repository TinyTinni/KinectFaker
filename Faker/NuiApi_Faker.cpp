#include "NuiApi_Faker.h"

#include <fstream> //fstream
#include <utility> //move
#include <vector>

//delete later
#include <iostream>

#include "NuiSensor_Faker.h"
#include "KinectFileDef.pb.h"

#include <boost/preprocessor/seq/for_each.hpp>

#include <string>

std::string fname = "";
int fake_index = -1;

//todo: multi devices
//struct FakeDevice
//{
//    const char* filename;
//    int idx;
//};
//
//std::vector<FakeDevice> currentDevices;

#define HOOKS \
    (NuiCreateSensorByIndex)\
    (NuiGetSensorCount)


//create global hooks with hook_<functionname> for each function in HOOKS
#define HOOKS_DEFINE_MACRO(r, data, elem)\
        subhook_t BOOST_PP_CAT(data, elem);
BOOST_PP_SEQ_FOR_EACH(HOOKS_DEFINE_MACRO, hook_, HOOKS);


//------------------------ Hook Functions ------------------- //
HRESULT my_NuiCreateSensorByIndex(_In_ int index, _Out_ INuiSensor ** ppNuiSensor)
{
    typedef HRESULT(*f)(int, INuiSensor**);
    if (index != fake_index)
        return ((f)subhook_get_trampoline(hook_NuiCreateSensorByIndex))(index, ppNuiSensor);


    std::ifstream scene_file(fname, std::ios::binary);
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
    //HRESULT r = ((f)subhook_get_trampoline(hook_NuiGetSensorCount))(pCount);
    //if (FAILED(r))
    //    return r;
    *pCount = 0;
    fake_index = (*pCount)++;
    return S_OK;
}


//------------------------ Lib Functions ------------------- //

HRESULT kinect_faker_init(const char* strFile)
{
    std::ifstream scene_file;
    scene_file.open(strFile, std::ios::in);
    if (!scene_file.is_open())
        return STG_E_FILENOTFOUND;
    fname = strFile;

    hook_NuiGetSensorCount = subhook_new((void*) NuiGetSensorCount, (void*)my_NuiGetSensorCount, SUBHOOK_OPTION_64BIT_OFFSET);
    hook_NuiGetSensorCount = subhook_new((void*)NuiCreateSensorByIndex, (void*)my_NuiCreateSensorByIndex, SUBHOOK_OPTION_64BIT_OFFSET);
    // install hooks

    //subhook_new((void*)elem, (void*)BOOST_PP_CAT(data, elem), SUBHOOK_OPTION_64BIT_OFFSET); /*todo: only for 64-bit*/\
    //call subhook_new with hooking function my_<functionName> for each function in HOOKS
#define HOOKS_INSTALL_MACRO(r, data, elem) BOOST_PP_CAT(hook_,elem) = \
        subhook_new((void*)elem,(void*)BOOST_PP_CAT(my_,elem),SUBHOOK_OPTION_64BIT_OFFSET);\
        subhook_install(BOOST_PP_CAT(hook_,elem));
    BOOST_PP_SEQ_FOR_EACH(HOOKS_INSTALL_MACRO, _, HOOKS);

    return S_OK;
}
void kinect_faker_release()
{
// call subhook_remove/free for each hook in HOOKS
#define HOOKS_UNINSTALL_MACRO(r, data, elem)\
        subhook_remove (BOOST_PP_CAT(hook_,elem));\
        subhook_free(BOOST_PP_CAT(hook_,elem));
    BOOST_PP_SEQ_FOR_EACH(HOOKS_UNINSTALL_MACRO, _, HOOKS);
}