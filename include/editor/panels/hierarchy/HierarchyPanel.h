// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include <QWidget>
class QTreeWidget;
namespace buffalo::editor
{
class SelectionService;
class SceneEditorController;
class HierarchyPanel final : public QWidget
{
public:
    HierarchyPanel(SceneEditorController& controller, SelectionService& selection, QWidget* parent = nullptr);

private:
    void Rebuild();
    void SyncSelection();
    SceneEditorController& controller_;
    SelectionService& selection_;
    QTreeWidget* tree_;
};
} // namespace buffalo::editor
