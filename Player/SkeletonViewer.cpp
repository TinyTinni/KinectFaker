#include "SkeletonViewer.h"

void SkeletonViewer::setSkeleton(const float* skd)
{
    m_skeletonVBO.bind();
    float* mapPtr = reinterpret_cast<float*>(m_skeletonVBO.map(QOpenGLBuffer::WriteOnly));
    memcpy(mapPtr, skd, m_skeletonVBO.size());
    m_skeletonVBO.unmap();
}

void SkeletonViewer::initializeGL()
{
    initializeOpenGLFunctions();

#ifdef _DEBUG
    m_debugLogger.initialize();
    connect(&m_debugLogger, &QOpenGLDebugLogger::messageLogged, [](const QOpenGLDebugMessage& msg)
    {
        qDebug() << msg.message();
    });
    m_debugLogger.startLogging();
#endif

    m_skeletonVBO = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_skeletonVBO.create();
    m_skeletonVBO.bind();
    m_skeletonVBO.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_skeletonVBO.allocate(NUI_SKELETON_POSITION_COUNT * 2*sizeof(float));

    m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
        R"(

attribute highp vec4 vertex;
void main()
{
    gl_Position = vertex;
}

        )");
    m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
        R"(

void main()
{
    gl_FragColor = vec4(0.0,1.0,0.0,0.0);
}

        )");
    m_shaderProgram.link();
    m_shaderProgram.bind();
}

void SkeletonViewer::resizeGL(int w, int h)
{
}

void SkeletonViewer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_skeletonVBO.bind();

    int vertexLocation = m_shaderProgram.attributeLocation("vertex");
    glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertexLocation);

    glDrawArrays(GL_POINTS, 0 , 20);

    glDisableVertexAttribArray(vertexLocation);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
