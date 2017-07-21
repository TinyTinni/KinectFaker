#pragma once

#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

#ifdef _DEBUG
#include <QOpenGLDebugLogger>
#endif

#include <NuiApi.h>

class SkeletonViewer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    SkeletonViewer(QWidget* parent = nullptr):QOpenGLWidget(parent){}
    void setSkeleton(const float* skd);

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();


private:
    QOpenGLBuffer m_skeletonVBO;
    QOpenGLShaderProgram m_shaderProgram;

#ifdef _DEBUG
    QOpenGLDebugLogger m_debugLogger;
#endif
};