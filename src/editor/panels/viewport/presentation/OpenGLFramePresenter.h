// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT
#pragma once

#include "render/RenderFrame.h"
#include <QImage>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <functional>
#include <memory>

class QOpenGLShaderProgram;
class QPainter;

namespace buffalo::editor
{
// Qt-only presentation adapter. It consumes an image, never a Scene.
class OpenGLFramePresenter final : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
public:
    explicit OpenGLFramePresenter(QWidget* parent);
    ~OpenGLFramePresenter() override;
    void SetFrame(const RenderFrame& frame, const QImage& image);
    void SetReadyCallback(std::function<void(QOpenGLContext*)> callback);
    void SetOverlay(std::function<void(QPainter&)> overlay);
    bool IsReady() const
    {
        return ready_;
    }

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    void ReleaseResources();
    std::unique_ptr<QOpenGLShaderProgram> program_;
    std::function<void(QOpenGLContext*)> readyCallback_;
    std::function<void(QPainter&)> overlay_;
    RenderFrame frame_;
    QImage image_;
    unsigned int vao_ = 0;
    bool ready_ = false;
};
} // namespace buffalo::editor