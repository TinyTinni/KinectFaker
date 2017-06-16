#pragma once

#include <subhook.h>
#include <Windows.h>
#include <NuiApi.h>

/** Initialize the Kinect Faker
    
    Sets up system hooks for all Nui*** functions, if necessary.


    @param strFile Filepath of the recorded KinectFile 

    @return S_OK if successfull, otherwise
        STG_E_FILENOTFOUND if file given by strFile could not be found
*/
HRESULT kinect_faker_init(const char* strFile);

/** Frees all ressources and remove all hooks

*/
void kinect_faker_release();

