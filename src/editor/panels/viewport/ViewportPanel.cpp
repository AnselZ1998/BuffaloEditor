// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/panels/viewport/ViewportPanel.h"
#include "editor/panels/viewport/ViewportSurface.h"
#include "editor/panels/viewport/ViewportToolbar.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <utility>

namespace buffalo::editor
{
ViewportPanel::ViewportPanel(IViewport& viewport, QWidget* parent)
    : QWidget(parent), viewport_(viewport)
{
    // -------------------- UI --------------------
    setObjectName("ViewportPanel");
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    auto* tools = new QHBoxLayout;
    tools->setContentsMargins(10, 6, 8, 6);
    auto* title = new QLabel("Viewport", this);
    title->setObjectName("ViewportTitle");
    tools->addWidget(title);
    tools->addStretch();
    auto* renderer = new QComboBox(this);
    renderer_ = renderer;
    renderer->setObjectName("RendererSelector");
    renderer->setToolTip("Renderer");
    for (std::size_t index = 0; index < viewport.BackendCount(); ++index)
    {
        renderer->addItem(QString::fromUtf8(viewport.BackendName(index)));
    }
    renderer->setCurrentIndex(viewport.BackendCount() > 0 ? static_cast<int>(viewport.ActiveBackend()) : -1);
    renderer->setEnabled(viewport.BackendCount() > 0);
    tools->addWidget(renderer);
    auto* frameAll = new QPushButton("Frame all  [F]", this);
    tools->addWidget(frameAll);
    layout->addLayout(tools);

    auto* body = new QHBoxLayout;
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(0);

    // toolbar & surface
    toolbar_ = new ViewportToolbar(viewport, this);
    surface_ = new ViewportSurface(viewport, this);
    body->addWidget(toolbar_);
    body->addWidget(surface_, 1);
    layout->addLayout(body, 1);
    auto* help = new QLabel("LMB / RMB: orbit    |    MMB: pan    |    Wheel: zoom    |    WASD: pan", this);
    help->setWordWrap(true);
    help->setObjectName("PanelHint");
    layout->addWidget(help);

    // -------------------- signal --------------------
    connect(renderer, &QComboBox::currentIndexChanged, this, [&viewport](int index)
    {
        if (index >= 0)
        {
            viewport.SelectBackend(static_cast<std::size_t>(index));
        }
    });

    connect(frameAll, &QPushButton::clicked, this, [&viewport]()
    {
        viewport.OnInput({InputAction::FrameAll});
    });
}

void ViewportPanel::Refresh()
{
    // GPU backends register only after the Qt share context exists.
    {
        const QSignalBlocker blocker(renderer_);
        if (renderer_->count() != static_cast<int>(viewport_.BackendCount()))
        {
            renderer_->clear();
            for (std::size_t index = 0; index < viewport_.BackendCount(); ++index)
            {
                renderer_->addItem(QString::fromUtf8(viewport_.BackendName(index)));
            }
        }
        renderer_->setEnabled(viewport_.BackendCount() > 0);
        renderer_->setCurrentIndex(viewport_.BackendCount() > 0 ? static_cast<int>(viewport_.ActiveBackend()) : -1);
    }
    toolbar_->Refresh();
    surface_->Refresh();
}
void ViewportPanel::SetOpenGLReadyCallback(std::function<void(QOpenGLContext*)> callback)
{
    surface_->SetOpenGLReadyCallback(std::move(callback));
}
} // namespace buffalo::editor
