// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include "editor/services/EditorLog.h"
#include <QWidget>

class QPlainTextEdit;

namespace buffalo::editor
{
class LogPanel final : public QWidget
{
public:
    explicit LogPanel(EditorLog& log, QWidget* parent = nullptr);
    void Refresh();
    void Clear();

private:
    EditorLog& log_;
    QPlainTextEdit* messages_ = nullptr;
    QString displayedText_;
    std::uint64_t revision_ = static_cast<std::uint64_t>(-1);
};
} // namespace buffalo::editor
