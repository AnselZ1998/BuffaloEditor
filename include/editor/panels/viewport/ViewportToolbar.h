// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include <QWidget>

class QToolButton;

namespace buffalo::editor
{
class IViewport;

class ViewportToolbar final : public QWidget
{
public:
    explicit ViewportToolbar(IViewport& viewport, QWidget* parent = nullptr);
    void Refresh();

private:
    IViewport& viewport_;
    QToolButton* gridButton_ = nullptr;
};
} // namespace buffalo::editor
