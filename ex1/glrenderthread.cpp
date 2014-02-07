#include <QGLShader>
#include <QFileInfo>

#include "glframe.h"

#include "glrenderthread.moc"
#include "glrenderthread.h"
#include <GL/glu.h>


QGLRenderThread::QGLRenderThread(QGLFrame *parent) :
    QThread(),
    GLFrame(parent) {
    doRendering = true;
    doResize = false;
    FrameCounter=0;

    ShaderProgram = NULL;
    VertexShader = GeometryShader = FragmentShader = NULL;
}

void QGLRenderThread::resizeViewport(const QSize &size) {
    w = size.width();
    h = size.height();
    doResize = true;
}

void QGLRenderThread::stop() {
    doRendering = false;
}


void QGLRenderThread::run() {
    GLFrame->makeCurrent();
    GLInit();
    LoadShader("Basic.vsh", "Basic.gsh", "Basic.fsh");

    while (doRendering) {
        if(doResize) {
            GLResize(w, h);
            doResize = false;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        paintGL(); // render actual frame

        FrameCounter++;
        GLFrame->swapBuffers();

        msleep(16); // wait 16ms => about 60 FPS
    }
}


void QGLRenderThread::GLInit(void) {
    glClearColor(0.25f, 0.25f, 0.4f, 0.0f);
}


void QGLRenderThread::GLResize(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    gluPerspective(45.,((GLfloat)width)/((GLfloat)height),0.1f,1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void QGLRenderThread::paintGL(void) {
    glTranslatef(0.0f, 0.0f, -5.0f);            // move 5 units into the screen
    glRotatef(FrameCounter,0.0f,0.0f,0.5f);     // rotate z-axis
    glBegin(GL_QUADS);
        glColor3f(1.,0.,0.);
        glVertex3f(-1.0, -1.0,0.0);
        glVertex3f(1.0, -1.0,0.0);
        glVertex3f(1.0, 1.0,0.0);
        glVertex3f(-1.0, 1.0,0.0);
    glEnd();
}


void QGLRenderThread::LoadShader(QString vshader, QString gshader, QString fshader) {
    if (ShaderProgram) {
        ShaderProgram->release();
        ShaderProgram->removeAllShaders();
    } else {
        ShaderProgram = new QGLShaderProgram;
    }

    if (VertexShader) {
        delete VertexShader;
        VertexShader = NULL;
    }

    if (GeometryShader) {
        delete GeometryShader;
        GeometryShader = NULL;
    }

    if (FragmentShader) {
        delete FragmentShader;
        FragmentShader = NULL;
    }

    // load and compile vertex shader
    QFileInfo vsh(vshader);
    if (vsh.exists()) {
        VertexShader = new QGLShader(QGLShader::Vertex);
        if (VertexShader->compileSourceFile(vshader)) {
            ShaderProgram->addShader(VertexShader);
        }
        else {
            qWarning() << "Vertex Shader Error" << VertexShader->log();
        }
    } else {
        qWarning() << "Vertex Shader source file " << vshader << " not found.";
    }

    qDebug() << "OpenGL Versions Supported: " << QGLFormat::openGLVersionFlags();
 
QString versionString(QLatin1String(reinterpret_cast<const char*>(glGetString(GL_VERSION))));
qDebug() << "Driver Version String:" << versionString;

    // load and compile geometry shader
    QFileInfo gsh(gshader);
    if (gsh.exists()) {
        GeometryShader = new QGLShader(QGLShader::Geometry);
        if (GeometryShader->compileSourceFile(gshader)) {
            ShaderProgram->addShader(GeometryShader);
        }
        else {
            qWarning() << "Geometry Shader Error" << GeometryShader->log();
        }
    } else {
        qWarning() << "Geometry Shader source file " << gshader << " not found.";
    }

    // load and compile fragment shader
    QFileInfo fsh(fshader);
    if (fsh.exists()) {
        FragmentShader = new QGLShader(QGLShader::Fragment);
        if (FragmentShader->compileSourceFile(fshader)) {
            ShaderProgram->addShader(FragmentShader);
        }
        else {
            qWarning() << "Fragment Shader Error" << FragmentShader->log();
        }
    } else {
        qWarning() << "Fragment Shader source file " << fshader << " not found.";
    }

    if (!ShaderProgram->link()) {
        qWarning() << "Shader Program Linker Error" << ShaderProgram->log();
    } else {
        ShaderProgram->bind();
    }
}