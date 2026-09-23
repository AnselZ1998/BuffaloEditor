// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT
#include "OpenGLFramePresenter.h"

#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QPainter>
#include <QSurfaceFormat>
#include <QDebug>
#include <utility>

namespace buffalo::editor
{
OpenGLFramePresenter::OpenGLFramePresenter(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setObjectName("OpenGLFramePresenter");
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFocusPolicy(Qt::NoFocus);
    QSurfaceFormat requested;
    requested.setVersion(3, 3);
    requested.setProfile(QSurfaceFormat::CoreProfile);
    requested.setDepthBufferSize(0);
    requested.setStencilBufferSize(0);
    setFormat(requested);
}

OpenGLFramePresenter::~OpenGLFramePresenter()
{
    if (context())
    {
        disconnect(context(), nullptr, this, nullptr);
    }
    ReleaseResources();
}

void OpenGLFramePresenter::ReleaseResources()
{
    ready_ = false;
    if (context() && context()->isValid())
    {
        makeCurrent();
        if (vao_)
        {
            glDeleteVertexArrays(1, &vao_);
        }
        program_.reset();
        doneCurrent();
    }
    vao_ = 0;
}

void OpenGLFramePresenter::SetReadyCallback(std::function<void(QOpenGLContext*)> callback)
{
    readyCallback_ = std::move(callback);
    if (ready_ && readyCallback_)
    {
        readyCallback_(context());
    }
}

void OpenGLFramePresenter::SetOverlay(std::function<void(QPainter&)> overlay)
{
    overlay_ = std::move(overlay);
}

void OpenGLFramePresenter::SetFrame(const RenderFrame& frame, const QImage& image)
{
    frame_ = frame;
    image_ = image;
    update();
}

void OpenGLFramePresenter::initializeGL()
{
    if (!initializeOpenGLFunctions())
    {
        qWarning("OpenGL 3.3 presentation unavailable; retaining software view.");
        return;
    }
    program_ = std::make_unique<QOpenGLShaderProgram>();
    const char* vertex = R"(
        #version 330 core
        out vec2 uv;
        void main() {
            vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
            uv = p;
            gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
        })";
    const char* fragment = R"(
        #version 330 core
        in vec2 uv;
        uniform sampler2D frameTexture;
        out vec4 color;
        void main() { color = texture(frameTexture, uv); })";
    if (!program_->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex) || !program_->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment) || !program_->link())
    {
        qWarning() << "OpenGL presentation shader failed:" << program_->log();
        program_.reset();
        return;
    }
    glGenVertexArrays(1, &vao_);
    ready_ = true;
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, [this]()
    {
        ReleaseResources();
    }, Qt::DirectConnection);
    if (readyCallback_)
    {
        readyCallback_(context());
        makeCurrent();
    }
}

void OpenGLFramePresenter::paintGL()
{
    if (!ready_)
    {
        return;
    }
    glViewport(0, 0, qRound(width() * devicePixelRatioF()), qRound(height() * devicePixelRatioF()));
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glClearColor(27.0f / 255.0f, 30.0f / 255.0f, 34.0f / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const bool gpuFrame = frame_.IsValid() && frame_.storage == FrameStorage::OpenGLTexture && frame_.graphicsDevice == context()->shareGroup();
    if (gpuFrame)
    {
        program_->bind();
        program_->setUniformValue("frameTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frame_.textureId);
        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        program_->release();
        // Borrowed producer texture may be reused by the next synchronous Render().
        // Finish the consumer too; a future asynchronous path should use fences.
        glFinish();
    }

    QPainter painter(this);
    if (!gpuFrame && !image_.isNull())
    {
        painter.drawImage(rect(), image_);
    }
    if (overlay_)
    {
        overlay_(painter);
    }
    if (!gpuFrame && image_.isNull())
    {
        painter.setPen(QColor("#a7afb9"));
        painter.drawText(rect().adjusted(16, 16, -16, -16), Qt::AlignCenter | Qt::TextWordWrap,
            "Waiting for a rendered frame. Check Log if the scene cannot load.");
    }
}
} // namespace buffalo::editor