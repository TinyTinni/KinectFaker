#include "main_window.h"

#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>

#include <fstream>

#include "Kinect.h"


main_window::main_window(QObject * parent)
{
    ui.setupUi(this);

    auto selectFileFn = [this](QLineEdit* le) 
    {
        QString str = QFileDialog::getOpenFileName(0, tr("Select File"));
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

    connect(ui.pbConnect, &QPushButton::clicked, this, &main_window::connect_kinect);
    connect(ui.pbRecord, &QPushButton::clicked, [this](bool checked)
    {
        if (!checked)
        {
            std::ofstream file(ui.leSkeletonOut->text().toLatin1(), std::ios::binary);
            m_scene.SerializePartialToOstream(&file);
            m_scene.Clear();
        }
    });

    connect(ui.cbEnableSmoothing, &QCheckBox::clicked, [this](bool checked)
    {
        if (checked)
            m_kinect->enable_smoothing(&m_smoothParams);
        else
            m_kinect->disable_smoothing();
    });

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
    connect(ui.pbPlaySkeletonPath, &QPushButton::clicked, [&selectFileFn, this]() {selectFileFn(ui.leSkeletonIn); });


    ////////////////// Skeleton Config ////////////////////////////
    connect(ui.spSmoothing, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        [this](double v) {this->m_smoothParams.fSmoothing = v; });
    connect(ui.spCorrection, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        [this](double v) {this->m_smoothParams.fCorrection = v; });
    connect(ui.spPrediction, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        [this](double v) {this->m_smoothParams.fPrediction = v; });
    connect(ui.spJitter, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        [this](double v) {this->m_smoothParams.fJitterRadius = v; });
    connect(ui.spMaxDev, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        [this](double v) {this->m_smoothParams.fMaxDeviationRadius = v; });
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
    currentDevice["connection_id"] = ui.leConnectionId->text();

    //todo handle empty stuff
    currentDevice["skeleton_file"] = ui.leSkeletonOut->text();
    currentDevice["color_file"] = ui.leColorOut->text();
    currentDevice["depth_file"] = ui.leDepthOut->text();

}

void main_window::connect_kinect(bool checked)
{
    //checked == true
    //////////////////////////////////////

    //init kinect, if neccessary
    if (!m_kinect)
    {
        m_skeletonEvent.setHandle(CreateEventW(NULL, TRUE, FALSE, NULL));
        m_skeletonEvent.setEnabled(true);
        m_kinect = std::make_unique<RecorderKinect>(m_skeletonEvent.handle());
        connect(&m_skeletonEvent, &QWinEventNotifier::activated, [this]() {m_kinect->event_next_frame_fired(); });

        BSTR connectionid = m_kinect->get_raw_device()->NuiDeviceConnectionId();
        ui.leConnectionId->setText(QString::fromWCharArray(connectionid));
        ui.pbGenerateConfig->setEnabled(true);
    }

    const auto showFN = [this](const NUI_SKELETON_FRAME& frame)
    {
        for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
        {
            const auto& skd = frame.SkeletonData[i];
            if (skd.eTrackingState == NUI_SKELETON_TRACKED)
            {
                std::vector<float> pos(2 * NUI_SKELETON_POSITION_COUNT); // for graphical output

                for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
                {
                    pos[2 * i] = skd.SkeletonPositions[i].x;
                    pos[2 * i + 1] = skd.SkeletonPositions[i].y;
                }
                this->m_skeletonViewer->setSkeleton(pos.data());
                this->m_skeletonViewer->update();
            }
        }
    };

    const auto recordFrameFN = [this](const NUI_SKELETON_FRAME& frame)
    {
        const auto VecVecCast = [](const Vector4& v)-> kif::Vector*
        {
            auto r = std::make_unique<kif::Vector>();
            r->set_x(v.x);
            r->set_y(v.y);
            r->set_z(v.z);
            r->set_w(v.w);
            return r.release();
        };

        kif::Frame* kif_frame = m_scene.add_frames();
        kif_frame->set_litimestamp(frame.liTimeStamp.QuadPart);
        kif_frame->set_dwframenumber(frame.dwFrameNumber);
        kif_frame->set_allocated_vfloorclipplane(VecVecCast(frame.vFloorClipPlane));
        kif_frame->set_allocated_vnormaltogravity(VecVecCast(frame.vNormalToGravity));

        const auto recordSkeletonFn = [](const NUI_SKELETON_DATA& skd, kif::SkeletonData* data)
        {
            data->set_etrackingstate(skd.eTrackingState);
            data->set_dwtrackingid(skd.dwTrackingID);
            data->set_dwenrollmentindex(skd.dwEnrollmentIndex);
            data->set_dwuserindex(skd.dwUserIndex);
            data->set_dwqualityflags(skd.dwQualityFlags);

            auto u_skel_pos = std::make_unique<kif::Vector>();
            u_skel_pos->set_x(skd.Position.x);
            u_skel_pos->set_y(skd.Position.y);
            u_skel_pos->set_z(skd.Position.z);
            u_skel_pos->set_w(skd.Position.w);
            data->set_allocated_position(u_skel_pos.release());


            for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
            {
                auto vec = data->add_skeletonpositions();
                vec->set_x(skd.SkeletonPositions[i].x);
                vec->set_y(skd.SkeletonPositions[i].y);
                vec->set_z(skd.SkeletonPositions[i].z);
                vec->set_w(skd.SkeletonPositions[i].w);

                data->add_eskeletonpositiontrackingstate(skd.eSkeletonPositionTrackingState[i]);
            }
        };

        for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
            recordSkeletonFn(frame.SkeletonData[i], kif_frame->add_skeleton_data());
    };

    const auto perFrameFN = [showFN, recordFrameFN, this](const NUI_SKELETON_FRAME& frame)
    {
        showFN(frame);
        if (this->ui.pbRecord->isChecked())
            recordFrameFN(frame);
    };

    const auto emptyFn = [](const NUI_SKELETON_FRAME&) {};

    std::function<void(const NUI_SKELETON_FRAME&)> newFrameCB = perFrameFN;
    if (!ui.cbCaptureSkeleton->isChecked()) newFrameCB = emptyFn; //trinary operator not possible

    m_kinect->set_new_point_callback(newFrameCB);
    m_kinect->enable();

    if (!m_kinect->isOn())
    {
        m_kinect.reset(nullptr);
        return;
    }

    m_kinect->start_record();
    ui.pbRecord->setEnabled(true);
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
    pos.reserve(2 * NUI_SKELETON_POSITION_COUNT);
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
