#pragma once

#include <memory>

#include "ui_main_window.h"
#include <QWinEventNotifier>

#include "Kinect.h"
#include "SkeletonViewer.h"

#include <KinectFileDef.pb.h>

class main_window : public QMainWindow
{
    Q_OBJECT
public:  

    main_window(QObject* parent = nullptr);
    ~main_window();

private:
    Ui::MainWindow ui;

    void generate_config();
    void connect_kinect(bool checked);

    void load_skeleton();
    void play_skeleton();
    void show_skeleton(int index);

    SkeletonViewer* m_skeletonViewer;

    kif::Scene m_scene;
    std::unique_ptr<RecorderKinect> m_kinect = nullptr;

    QWinEventNotifier m_skeletonEvent;
    NUI_TRANSFORM_SMOOTH_PARAMETERS m_smoothParams;
};