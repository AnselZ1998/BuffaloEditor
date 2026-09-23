// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include "editor/viewport/IViewport.h"
#include <QElapsedTimer>
#include <QImage>
#include <QSet>
#include <QWidget>
#include <functional>

class QPainter;
class QLabel;
class QOpenGLContext;
namespace buffalo::editor
{
class OpenGLFramePresenter;
// Qt input and presentation only. The UI never sees Scene, Camera or a backend.
class ViewportSurface final : public QWidget
{
public:
    explicit ViewportSurface(IViewport& viewport, QWidget* parent = nullptr);
    void Refresh();
    void SetOpenGLReadyCallback(std::function<void(QOpenGLContext*)> callback);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    static bool IsNavigationKey(int key);
    void EndNavigation();
    void DrawEditorGrid(QPainter& painter);

    IViewport& viewport_;
    QImage image_;
    QElapsedTimer clock_;
    QSet<int> keys_;
    QPointF lastPosition_;
    bool navigating_ = false;
    bool dragging_ = false;
    QLabel* fpsLabel_ = nullptr;
    OpenGLFramePresenter* glPresenter_ = nullptr;
};
} // namespace buffalo::editor
