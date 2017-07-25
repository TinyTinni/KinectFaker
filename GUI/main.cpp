#include <Windows.h>
#include <KinectFileDef.pb.h> //GOOGLE_PROTOBUF_VERIFY_VERSION

#include <QSurfaceFormat>
#include "main_window.h"

int main(int argc, char *argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    QSurfaceFormat glContextOptions;
#ifdef _DEBUG
    glContextOptions.setOption(QSurfaceFormat::DebugContext);
#endif
    glContextOptions.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(glContextOptions);

    QApplication app{ argc,argv };
    main_window m;
    m.show();
    return app.exec();
}