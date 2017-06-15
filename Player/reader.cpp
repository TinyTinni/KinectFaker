#include "reader.h"


#include <fstream>

void get_point(const ksk::SkeletonData::Vector3& in, float* data)
{
    constexpr int        cScreenWidth = 320;
    constexpr int        cScreenHeight = 240;
    *data = 0.5f*(-1.f + 2.f*static_cast<float>(in.x())*1.f / cScreenWidth);
    ++data;
    *data = 0.5f*(1.f - 2.f*static_cast<float>(in.y())*1.f / cScreenHeight);
    ++data;
    *data = 0.f;//ingore depth value
    ++data;
}

bool Reader::get_skeleton_position(float * data)
{
    if (m_currentFrame == m_scene.frames().cend())
        m_currentFrame = m_scene.frames().cbegin();


    const ksk::Frame& fr = *m_currentFrame;
    
    for (const auto& sk : fr.skeleton_data())
    {
        int i = 0;
        for (const auto& j : sk.joints())
        {
            get_point(j, &data[i * 2 * 3]);
            ++i;
        }

        auto cpy = [&data](const int srcIdx, const int dstIdx)
        {
            memcpy(&data[3 * (2 * dstIdx + 1)], &data[3 * 2 * srcIdx], 3 * sizeof(float));
        };


        cpy(NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_HEAD);//redundant copy, maybe you should delete it if needed

        cpy(NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER);
        cpy(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT);
        cpy(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT);
        cpy(NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE);
        cpy(NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER);
        cpy(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT);
        cpy(NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT);

        // Left Arm
        cpy(NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT);
        cpy(NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT);
        cpy(NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);

        // Right Arm
        cpy(NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT);
        cpy(NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT);
        cpy(NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);

        // Left Leg
        cpy(NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT);
        cpy(NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT);
        cpy(NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);

        // Right Leg
        cpy(NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT);
        cpy(NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT);
        cpy(NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);
    }
    Sleep(30);
    ++m_currentFrame;
    return true;
}

Reader::Reader()
{
    std::ifstream file("test_file.ksk");
    m_scene.ParseFromIstream(&file);
    m_currentFrame = m_scene.frames().cbegin();
}