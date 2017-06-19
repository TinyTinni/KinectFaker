#pragma once

#include <subhook.h>
#include <Windows.h>
#include <NuiApi.h>

/** Initialize the Kinect Faker
    
    Sets up system hooks for all Nui*** functions, if necessary.

    maybe deprecated, cannot decide

    @param strFile Filepath of the recorded KinectFile 

    @return S_OK if successfull, otherwise
        STG_E_FILENOTFOUND if file given by strFile could not be found
*/
HRESULT kinect_faker_init(const char* strFile);


/** Frees all ressources and remove all hooks

maybe deprecated, cannot decide
*/
void kinect_faker_release();

struct FakeDevice;
typedef FakeDevice* fake_kinect_t;

fake_kinect_t fake_kinect_add(const char* strFile);
void fake_kinect_free(fake_kinect_t& hnd);