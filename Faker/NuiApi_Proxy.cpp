#include "NuiApi_Proxy.h"
#include <NuiApi.h>

HRESULT proxy::NuiTransformSmooth(NUI_SKELETON_FRAME *pSkeletonFrame, const NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothingParams)
{
    return ::NuiTransformSmooth(pSkeletonFrame, pSmoothingParams);
}

void proxy::NuiTransformSkeletonToDepthImage(
    Vector4 vPoint,
    LONG *plDepthX,
    LONG *plDepthY,
    USHORT *pusDepthValue
)
{
    ::NuiTransformSkeletonToDepthImage(vPoint, plDepthX, plDepthY, pusDepthValue);
}

HRESULT proxy::NuiCreateSensorByIndex(
    int index,
    INuiSensor **ppNuiSensor
)
{
    return ::NuiCreateSensorByIndex(index, ppNuiSensor);
}