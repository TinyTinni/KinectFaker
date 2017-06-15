#pragma once

#include <Windows.h>
#include <NuiApi.h>
#include <KinectFileDef.pb.h>


constexpr int skeleton_joints = NUI_SKELETON_POSITION_COUNT;

class Reader
{
    ksk::Scene m_scene;
    google::protobuf::RepeatedPtrField<ksk::Frame>::const_iterator m_currentFrame;
public:
    bool get_skeleton_position(float* data);
    Reader();
};