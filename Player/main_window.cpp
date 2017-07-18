#include "main_window.h"

#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>

#include <fstream>

#include "Kinect.h"


main_window::main_window(QObject * parent)
{
    ui.setupUi(this);

    auto selectFileFn = [=](QLineEdit* le) 
    {
        QString str = QFileDialog::getOpenFileName(this, tr("Save File to"), "", ".wmv");
        le->setText(str);
    };

    ////////////////// Record Tab ////////////////////////////
    ui.leColorOut->setText("ColorStream.wmv");
    ui.leDepthOut->setText("DepthStream.wmv");
    ui.leSkeletonOut->setText("Skeleton.kif");
    ui.leConnectionId->setText("<Not Connected>");

    connect(ui.pbFileColor, &QPushButton::clicked, [&selectFileFn, this]() {selectFileFn(ui.leColorOut); });
    connect(ui.pbFileDepth, &QPushButton::clicked, [&selectFileFn, this]() {selectFileFn(ui.leDepthOut); });
    connect(ui.pbFileSkeleton, &QPushButton::clicked, [&selectFileFn, this]() {selectFileFn(ui.leSkeletonOut); });

    connect(ui.pbGenerateConfig, &QPushButton::clicked, this, &main_window::generate_config);

    connect(ui.pbRecord, &QPushButton::clicked, this, &main_window::start_record);

    //unify size of group boxes
    //const auto groupBoxSize = ui.boxColorStream->size.width() + ui.boxDepthStream->size.width() + ui.boxSkeletonStream->size.width();
    
    ui.boxSkeletonStream->setLayout(new QGridLayout());
    m_skeletonViewer = new SkeletonViewer();
    ui.boxSkeletonStream->layout()->addWidget(m_skeletonViewer);
    //new SkeletonViewer(ui.boxSkeletonStream);
    //ui.streamSplitter->adjustSize();
    auto sizes = ui.streamSplitter->sizes();
    sizes.replace(0, ui.previewWidget->width() / 3);
    sizes.replace(1, ui.previewWidget->width() / 3);
    sizes.replace(2, ui.previewWidget->width() / 3);
    ui.streamSplitter->setSizes(sizes);

    ////////////////// Play Tab ////////////////////////////
    connect(ui.pbPlaySkeleton, &QPushButton::clicked, this, &main_window::play_skeleton);
    connect(ui.pbLoad, &QPushButton::clicked, this, &main_window::load_skeleton);
    connect(ui.hsFrames, &QSlider::valueChanged, this, &main_window::show_skeleton);

}

main_window::~main_window()
{
}

void main_window::generate_config()
{
    QJsonObject root;
    //logger
    QJsonObject logger;
    root["logger"] = logger;
    //logger level    
    //    trace = 0,
    //    debug = 1,
    //    info = 2,
    //    warn = 3,
    //    err = 4,
    //    critical = 5,
    //    off = 6
    logger["level"] = ui.cbLogLevel->currentIndex();
    logger["trace_calls"] = false;

    QJsonObject devices;
    root["devices"] = devices;

    QJsonObject currentDevice;// support multiple kinect devices
    devices["device1"] = currentDevice;
    currentDevice["ConnectionId"] = ui.leConnectionId->text();

    //todo handle empty stuff
    currentDevice["skeleton_file"] = ui.leSkeletonOut->text();
    currentDevice["color_file"] = ui.leColorOut->text();
    currentDevice["depth_file"] = ui.leDepthOut->text();

}

void main_window::start_record(bool checked)
{
    if (!checked)
    {
        m_skeletonEvent.setEnabled(false);
        m_kinect->stop_record();
        std::ofstream file(ui.leSkeletonOut->text().toLatin1(), std::ios::binary);
        m_scene.SerializePartialToOstream(&file);
        m_scene.Clear();
        return;
    }

    //checked == true
    if (!m_kinect)
    {
        m_skeletonEvent.setHandle(CreateEventW(NULL, TRUE, FALSE, NULL));
        m_skeletonEvent.setEnabled(true);
        m_kinect = std::make_unique<RecorderKinect>(m_skeletonEvent.handle());
        connect(&m_skeletonEvent, &QWinEventNotifier::activated, [this]() {m_kinect->event_next_frame_fired(); });
    }

    const auto recordSkeletonFn = [this](const NUI_SKELETON_DATA& skd) 
    {
        kif::Frame* frame = m_scene.add_frames();
        kif::SkeletonData* data = frame->add_skeleton_data();
        data->set_etrackingstate(skd.eTrackingState);

        std::vector<float> pos(2 * NUI_SKELETON_POSITION_COUNT);

        for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
        {
            auto vec = data->add_skeletonpositions();
            vec->set_x(skd.SkeletonPositions[i].x);
            vec->set_y(skd.SkeletonPositions[i].y);
            vec->set_z(skd.SkeletonPositions[i].z);
            vec->set_w(skd.SkeletonPositions[i].w);
            //std::cout << vec->x() << "\t" << vec->y() << "\t" << vec->z() << std::endl;
            data->add_eskeletonpositiontrackingstate(skd.eSkeletonPositionTrackingState[i]);

            pos[2 * i] = vec->x();
            pos[2 * i + 1] = vec->y();
        }
        m_skeletonViewer->setSkeleton(pos.data());
        m_skeletonViewer->update();
    };
    const auto emptyFn = [](const NUI_SKELETON_DATA& skd) {};

    std::function<void(const NUI_SKELETON_DATA&)> newFrameCB = recordSkeletonFn;
    if (!ui.cbCaptureSkeleton->isChecked()) newFrameCB = emptyFn; //trinary operator not possible

    m_kinect->set_new_frame_callback([]() {});
    m_kinect->set_new_point_callback(newFrameCB);

    while (!m_kinect->isOn())
        m_kinect->enable();

    m_kinect->start_record();
    m_skeletonEvent.setEnabled(true);
    return;

}

void main_window::load_skeleton()
{
    std::ifstream file(ui.leSkeletonIn->text().toLatin1(), std::ios::binary);
    const bool ok = m_scene.ParseFromIstream(&file); // todo: error detection

    if (!ok)
    {
        ui.hsFrames->setDisabled(true);

        //return;
    }

    const size_t num_frames = m_scene.frames_size()-1;
    ui.labelFramesUntil->setText(QString("%1").arg(num_frames));
    ui.hsFrames->setEnabled(true);
    ui.hsFrames->setMaximum((int)num_frames);
    ui.hsFrames->setValue(0);
}

void main_window::play_skeleton()
{
    std::ifstream file(ui.leSkeletonIn->text().toLatin1());
    m_scene.ParseFromIstream(&file); // todo: error detection

}

void main_window::show_skeleton(int index)
{
    auto frame = m_scene.frames(index);
    if (frame.skeleton_data_size() == 0)
        return;
    auto skd = frame.skeleton_data(0);

    std::vector<float> pos;
    pos.reserve(4 * NUI_SKELETON_POSITION_COUNT);
    for (auto v : skd.skeletonpositions())
    {
        pos.push_back(v.x());
        pos.push_back(v.y());
        //pos.push_back(0.f);
        //pos.push_back(0.f);
    }
    m_skeletonViewer->setSkeleton(pos.data());
    m_skeletonViewer->update();
}
