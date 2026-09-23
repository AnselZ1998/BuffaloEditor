// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include <QWidget>
#include <functional>

class QComboBox;
class QOpenGLContext;

namespace buffalo::editor
{
class IViewport;
class ViewportSurface;
class ViewportToolbar;

class ViewportPanel final : public QWidget
{
public:
    explicit ViewportPanel(IViewport& viewport, QWidget* parent = nullptr);
    void Refresh();
    void SetOpenGLReadyCallback(std::function<void(QOpenGLContext*)> callback);

private:
    IViewport& viewport_;
    QComboBox* renderer_ = nullptr;
    ViewportSurface* surface_ = nullptr;
    ViewportToolbar* toolbar_ = nullptr;
};
} // namespace buffalo::editor
