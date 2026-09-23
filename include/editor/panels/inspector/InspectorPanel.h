// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include <QWidget>

namespace buffalo::editor
{
class SelectionService;

class InspectorPanel final : public QWidget
{
public:
    explicit InspectorPanel(SelectionService& selection, QWidget* parent = nullptr);
};
} // namespace buffalo::editor
