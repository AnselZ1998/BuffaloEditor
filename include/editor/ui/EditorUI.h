// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include <QByteArray>
#include <QMainWindow>

class QMenu;
class QSettings;
class QToolBar;

namespace buffalo::editor
{
// Pure QMainWindow shell: accepts widgets/actions, never constructs panels.
class EditorUI final : public QMainWindow
{
public:
    explicit EditorUI(QWidget* parent = nullptr);
    QDockWidget* AddDock(const QString& title, const QString& name, QWidget* panel, Qt::DockWidgetArea area);
    void SetCentralWidget(QWidget* widget);
    void AddWorkspaceAction(QAction* action);
    void CaptureDefaultLayout();
    void RestoreLayout(QSettings& settings);
    void SaveLayout(QSettings& settings) const;
    void ResetLayout();

private:
    QMenu* windowMenu_ = nullptr;
    QToolBar* workspaceToolbar_ = nullptr;
    QByteArray defaultLayout_;
};
} // namespace buffalo::editor
