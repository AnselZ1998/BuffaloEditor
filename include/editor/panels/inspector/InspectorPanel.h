// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include <QWidget>
class QScrollArea;
class QLabel;
namespace buffalo::editor
{
class SelectionService;
class SceneEditorController;
class InspectorPanel final : public QWidget
{
public:
    InspectorPanel(SceneEditorController& controller, SelectionService& selection, QWidget* parent = nullptr);

private:
    void Rebuild();
    SceneEditorController& controller_;
    SelectionService& selection_;
    QScrollArea* scroll_;
    QLabel* title_;
};
} // namespace buffalo::editor
