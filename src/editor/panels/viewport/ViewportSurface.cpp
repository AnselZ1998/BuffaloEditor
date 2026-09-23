// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/panels/viewport/ViewportSurface.h"
#include "presentation/OpenGLFramePresenter.h"

#include <QGuiApplication>
#include <QResizeEvent>

#include <QLabel>
#include <QFocusEvent>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <algorithm>
#include <utility>

namespace buffalo::editor
{
ViewportSurface::ViewportSurface(IViewport& viewport, QWidget* parent)
    : QWidget(parent), viewport_(viewport)
{
    setObjectName("ViewportImage");
    setFocusPolicy(Qt::StrongFocus);
    fpsLabel_ = new QLabel("FPS: --", this);
    fpsLabel_->setObjectName("ViewportFPS");
    fpsLabel_->setAlignment(Qt::AlignCenter);
    fpsLabel_->setFixedSize(80, 24);
    setMinimumSize(160, 120);
    setAttribute(Qt::WA_OpaquePaintEvent);
    clock_.start();
    // Headless Qt plugins cannot create QOpenGLWidget contexts. Keep the existing
    // raster QWidget path for tests and machines where GPU presentation is disabled.
    const QString platform = QGuiApplication::platformName();
    if (platform != "offscreen" && platform != "minimal")
    {
        glPresenter_ = new OpenGLFramePresenter(this);
        glPresenter_->setGeometry(0, 0, 1, 1);
        glPresenter_->SetOverlay([this](QPainter& painter)
        {
            if (viewport_.GetShowFlags().grid)
            {
                DrawEditorGrid(painter);
            }
        });
        fpsLabel_->raise();
    }
}

void ViewportSurface::Refresh()
{
    const float deltaTime = std::min(0.1f, static_cast<float>(clock_.restart()) / 1000.0f);
    if (!isVisible() || window()->isMinimized() || width() <= 0 || height() <= 0)
    {
        EndNavigation();
        viewport_.SetSize(0, 0);
        image_ = QImage();
        return;
    }

    if (deltaTime > 0.0f)
    {
        const float fps = 1.0f / deltaTime;
        fpsLabel_->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
    }

    if (hasFocus())
    {
        const float x = static_cast<float>(keys_.contains(Qt::Key_A)) - static_cast<float>(keys_.contains(Qt::Key_D));
        const float y = static_cast<float>(keys_.contains(Qt::Key_W)) - static_cast<float>(keys_.contains(Qt::Key_S));
        if (x != 0.0f || y != 0.0f)
        {
            viewport_.OnInput({InputAction::Pan, x * deltaTime * 0.5f, y * deltaTime * 0.5f});
            navigating_ = true;
        }
    }

    viewport_.SetSize(qRound(width() * devicePixelRatioF()), qRound(height() * devicePixelRatioF()));
    const ViewportFrame frame = viewport_.RenderFrame();
    if (frame.IsValid() && frame.storage == FrameStorage::CpuPixels && frame.format == PixelFormat::BGRA8)
    {
        // ARGB32 is BGRA byte order on Windows. Copy before the renderer can reuse its buffer.
        const QImage borrowed(reinterpret_cast<const uchar*>(frame.pixels), frame.width, frame.height, frame.rowPitchBytes, QImage::Format_ARGB32);
        image_ = borrowed.copy();
    }
    else
    {
        image_ = QImage();
    }
    if (glPresenter_)
    {
        glPresenter_->SetFrame(frame, image_);
        if (glPresenter_->IsReady())
        {
            glPresenter_->setGeometry(rect());
            glPresenter_->show();
            fpsLabel_->raise();
        }
        else
        {
            glPresenter_->hide();
        }
    }
    update();
}

void ViewportSurface::SetOpenGLReadyCallback(std::function<void(QOpenGLContext*)> callback)
{
    if (glPresenter_)
    {
        glPresenter_->SetReadyCallback(std::move(callback));
    }
}

void ViewportSurface::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (glPresenter_ && glPresenter_->IsReady())
    {
        glPresenter_->setGeometry(rect());
    }
    fpsLabel_->raise();
}

void ViewportSurface::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#1b1e22"));
    if (!image_.isNull())
    {
        painter.drawImage(rect(), image_);
    }
    if (viewport_.GetShowFlags().grid)
    {
        DrawEditorGrid(painter);
    }
    if (image_.isNull())
    {
        painter.setPen(QColor("#a7afb9"));
        painter.drawText(rect().adjusted(16, 16, -16, -16), Qt::AlignCenter | Qt::TextWordWrap, "Waiting for a rendered frame. Check Log if the scene cannot load.");
    }
}

void ViewportSurface::DrawEditorGrid(QPainter& painter)
{
    // v1 screen-space perspective guide: no Scene mesh, picking or bounds.
    // It deliberately has no world coordinates, camera tracking or depth test.
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal horizon = height() * 0.38;
    const QPointF vanishingPoint(width() * 0.5, horizon);
    painter.setPen(QPen(QColor(135, 144, 154, 48), 1.0));
    for (int line = -8; line <= 8; ++line)
    {
        const qreal x = width() * 0.5 + line * width() * 0.22;
        painter.drawLine(vanishingPoint, QPointF(x, height()));
    }
    for (int line = 1; line <= 12; ++line)
    {
        const qreal fraction = line / 12.0;
        const qreal y = horizon + fraction * fraction * (height() - horizon);
        painter.drawLine(QPointF(0, y), QPointF(width(), y));
    }
    painter.restore();
}

void ViewportSurface::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton || event->button() == Qt::MiddleButton)
    {
        setFocus(Qt::MouseFocusReason);
        lastPosition_ = event->position();
        dragging_ = true;
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void ViewportSurface::mouseMoveEvent(QMouseEvent* event)
{
    const QPointF delta = event->position() - lastPosition_;
    lastPosition_ = event->position();
    if (!dragging_ || height() <= 0 || delta.isNull())
    {
        return;
    }
    const float x = static_cast<float>(delta.x()) / static_cast<float>(height());
    const float y = static_cast<float>(delta.y()) / static_cast<float>(height());
    if (event->buttons() & Qt::MiddleButton)
    {
        viewport_.OnInput({InputAction::Pan, x, y});
    }
    else if (event->buttons() & (Qt::LeftButton | Qt::RightButton))
    {
        viewport_.OnInput({InputAction::Orbit, x, y});
    }
    else
    {
        return;
    }
    navigating_ = true;
    event->accept();
}

void ViewportSurface::mouseReleaseEvent(QMouseEvent* event)
{
    if (!(event->buttons() & (Qt::LeftButton | Qt::RightButton | Qt::MiddleButton)))
    {
        dragging_ = false;
        if (keys_.isEmpty())
        {
            EndNavigation();
        }
    }
    event->accept();
}

void ViewportSurface::wheelEvent(QWheelEvent* event)
{
    const float steps = static_cast<float>(event->angleDelta().y()) / 120.0f;
    if (steps != 0.0f)
    {
        viewport_.OnInput({InputAction::Zoom, steps});
    }
    event->accept();
}

void ViewportSurface::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_F)
    {
        if (!event->isAutoRepeat())
        {
            viewport_.OnInput({InputAction::FrameAll});
        }
    }
    else if (IsNavigationKey(event->key()))
    {
        keys_.insert(event->key());
    }
    else
    {
        QWidget::keyPressEvent(event);
        return;
    }
    event->accept();
}

void ViewportSurface::keyReleaseEvent(QKeyEvent* event)
{
    if (IsNavigationKey(event->key()))
    {
        if (!event->isAutoRepeat())
        {
            keys_.remove(event->key());
            if (keys_.isEmpty() && !dragging_)
            {
                EndNavigation();
            }
        }
        event->accept();
        return;
    }
    QWidget::keyReleaseEvent(event);
}

void ViewportSurface::focusOutEvent(QFocusEvent* event)
{
    EndNavigation();
    QWidget::focusOutEvent(event);
}

void ViewportSurface::hideEvent(QHideEvent* event)
{
    EndNavigation();
    viewport_.SetSize(0, 0);
    image_ = QImage();
    QWidget::hideEvent(event);
}

bool ViewportSurface::IsNavigationKey(int key)
{
    return key == Qt::Key_W || key == Qt::Key_A || key == Qt::Key_S || key == Qt::Key_D;
}

void ViewportSurface::EndNavigation()
{
    if (navigating_)
    {
        viewport_.OnInput({InputAction::NavigationEnd});
    }
    navigating_ = false;
    dragging_ = false;
    keys_.clear();
}
} // namespace buffalo::editor
