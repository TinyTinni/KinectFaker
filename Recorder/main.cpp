#include <KinectFileDef.pb.h>

#include <fstream>
#include <iostream>

#include "Kinect/Kinect.h"

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    {
        //write output

        ksk::Scene scene;
        ksk::Frame* frame;
        ksk::SkeletonData* data;
        auto newFrame = [&scene, &frame, &data]()
        {
            frame = scene.add_frames();
            data = frame->add_skeleton_data();
        };
        auto newPointCB = [&data](size_t id, long x, long y, unsigned short depth)
        {
            ksk::SkeletonData::Vector3* vec3 = data->add_joints();
            vec3->set_x(x);
            vec3->set_y(y);
            vec3->set_z(depth);
        };

        Kinect kinect;
        kinect.set_new_frame_callback(newFrame);
        kinect.set_new_point_callback(newPointCB);

        while (!kinect.isOn())
            kinect.enable();

        kinect.start_record();

        int captureFrames = 150;

        for (; captureFrames; --captureFrames)
            while (!kinect.get_skeleton_position());

        kinect.stop_record();

        std::ofstream file("test_file.ksk");
        scene.SerializePartialToOstream(&file);
    }

    google::protobuf::ShutdownProtobufLibrary();

    system("Pause");

    return 0;
}