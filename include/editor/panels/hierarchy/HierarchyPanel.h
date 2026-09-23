// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include <QWidget>

namespace buffalo::editor
{
class SelectionService;

class HierarchyPanel final : public QWidget
{
public:
    HierarchyPanel(SelectionService& selection, bool sceneReady, QWidget* parent = nullptr);
};
} // namespace buffalo::editor
