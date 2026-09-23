// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/ui/EditorUI.h"
#include "editor/ui/EditorTheme.h"

#include <QDockWidget>
#include <QLabel>
#include <QMenuBar>
#include <QSettings>
#include <QStatusBar>
#include <QToolBar>

namespace buffalo::editor
{
EditorUI::EditorUI(QWidget* parent)
    : QMainWindow(parent)
{
    setObjectName("BuffaloEditor");
    setWindowTitle("Buffalo Editor");
    resize(1440, 900);
    setMinimumSize(960, 640);
    setDockNestingEnabled(true);
    setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
    auto* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("E&xit", this, &QWidget::close);
    windowMenu_ = menuBar()->addMenu("&Window");
    windowMenu_->addAction("Reset layout", this, &EditorUI::ResetLayout);
    windowMenu_->addSeparator();

    workspaceToolbar_ = addToolBar("Workspace");
    workspaceToolbar_->setObjectName("WorkspaceToolbar");
    workspaceToolbar_->setMovable(false);
    const QIcon logo = EditorTheme::Logo();
    if (!logo.isNull())
    {
        setWindowIcon(logo);
        auto* image = new QLabel(workspaceToolbar_);
        image->setPixmap(logo.pixmap(32, 32));
        workspaceToolbar_->addWidget(image);
    }
    auto* brand = new QLabel("  BUFFALO  ", workspaceToolbar_);
    brand->setObjectName("BuffaloBrand");
    workspaceToolbar_->addWidget(brand);
    workspaceToolbar_->addSeparator();
    statusBar()->showMessage("Ready");
    statusBar()->addPermanentWidget(new QLabel("Buffalo  |  v1", this));
}

QDockWidget* EditorUI::AddDock(const QString& title, const QString& name, QWidget* panel, Qt::DockWidgetArea area)
{
    auto* dock = new QDockWidget(title, this);
    dock->setObjectName(name);
    dock->setWidget(panel);
    addDockWidget(area, dock);
    windowMenu_->addAction(dock->toggleViewAction());
    return dock;
}

void EditorUI::SetCentralWidget(QWidget* widget)
{
    setCentralWidget(widget);
}

void EditorUI::AddWorkspaceAction(QAction* action)
{
    workspaceToolbar_->addAction(action);
}

void EditorUI::CaptureDefaultLayout()
{
    defaultLayout_ = saveState(1);
}

void EditorUI::RestoreLayout(QSettings& settings)
{
    restoreGeometry(settings.value("geometry").toByteArray());
    if (!restoreState(settings.value("layout").toByteArray(), 1))
    {
        ResetLayout();
    }
}

void EditorUI::SaveLayout(QSettings& settings) const
{
    settings.setValue("geometry", saveGeometry());
    settings.setValue("layout", saveState(1));
}

void EditorUI::ResetLayout()
{
    if (!defaultLayout_.isEmpty())
    {
        restoreState(defaultLayout_, 1);
    }
}
} // namespace buffalo::editor
