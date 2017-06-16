#pragma once

//#include <BaseTsd.h> // types like USHORT etc.
//#include <WinDef.h> // HRESULT

#include <Windows.h>

struct _NUI_SKELETON_FRAME;
struct _NUI_TRANSFORM_SMOOTH_PARAMETERS;
struct _Vector4;
struct INuiSensor;

namespace proxy
{
    HRESULT NuiTransformSmooth(
        struct _NUI_SKELETON_FRAME *pSkeletonFrame,
        const struct _NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothingParams
    );

    void NuiTransformSkeletonToDepthImage(
        struct _Vector4 vPoint,
        LONG *plDepthX,
        LONG *plDepthY,
        USHORT *pusDepthValue
    );

    HRESULT NuiCreateSensorByIndex(
        int index,
        INuiSensor **ppNuiSensor
    );

}