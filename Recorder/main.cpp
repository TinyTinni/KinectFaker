#include <KinectFileDef.pb.h>

#include <fstream>
#include <iostream>

#include "Kinect/Kinect.h"

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    {
        //write output

        kif::Scene scene;
        kif::Frame* frame;
        auto newFrame = [&scene, &frame]()
        {
            frame = scene.add_frames();
        };
        auto newPointCB = [&frame](const NUI_SKELETON_DATA& skd)
        {
            kif::SkeletonData* data = frame->add_skeleton_data();
            data->set_etrackingstate(skd.eTrackingState);

            for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
            {
                auto vec = data->add_skeletonpositions();
                vec->set_x(skd.SkeletonPositions[i].x);
                vec->set_y(skd.SkeletonPositions[i].y);
                vec->set_z(skd.SkeletonPositions[i].z);
                vec->set_w(skd.SkeletonPositions[i].w);
                //std::cout << vec->x() << "\t" << vec->y() << "\t" << vec->z() << std::endl;
                data->add_eskeletonpositiontrackingstate(skd.eSkeletonPositionTrackingState[i]);
            }
        };

        Kinect kinect;
        kinect.set_new_frame_callback(newFrame);
        kinect.set_new_point_callback(newPointCB);

        while (!kinect.isOn())
            kinect.enable();

        kinect.start_record();

        while ( scene.frames_size() != 150)
            while (!kinect.get_skeleton_position());

        kinect.stop_record();

        std::ofstream file("test_file.kif", std::ios::binary);
        scene.SerializePartialToOstream(&file);

        std::cout << "cauptured frames: " << scene.frames_size() << std::endl;

    }

    google::protobuf::ShutdownProtobufLibrary();
    system("Pause");

    return 0;
}