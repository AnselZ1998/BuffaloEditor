// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/panels/viewport/ViewportToolbar.h"
#include "editor/viewport/IViewport.h"

#include <QIcon>
#include <QSize>
#include <QSignalBlocker>
#include <QToolButton>
#include <QVBoxLayout>

namespace buffalo::editor
{
ViewportToolbar::ViewportToolbar(IViewport& viewport, QWidget* parent)
    : QWidget(parent), viewport_(viewport)
{

    setObjectName("ViewportToolbar");
    setFixedWidth(40);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 6, 4, 6);
    layout->setSpacing(4);

    // -------------------- Grid UI --------------------
    gridButton_ = new QToolButton(this);
    gridButton_->setObjectName("GridToggle");
    gridButton_->setIcon(QIcon(":/buffalo/icons/grid.png"));
    gridButton_->setIconSize(QSize(20, 20));
    gridButton_->setFixedSize(32, 32);
    gridButton_->setAccessibleName("Show grid");
    gridButton_->setToolTip("Show editor grid (screen-space guide)");
    gridButton_->setCheckable(true);
    gridButton_->setChecked(viewport_.GetShowFlags().grid);
    layout->addWidget(gridButton_);
    // Add future visualization buttons here, before the stretch.
    layout->addStretch();

    // -------------------- Grid signal --------------------
    connect(gridButton_, &QToolButton::toggled, this, [this](bool checked)
    {
        auto flags = viewport_.GetShowFlags();
        flags.grid = checked;
        viewport_.SetShowFlags(flags);
    });

}

void ViewportToolbar::Refresh()
{
    const QSignalBlocker blocker(gridButton_);
    gridButton_->setChecked(viewport_.GetShowFlags().grid);
}
} // namespace buffalo::editor
