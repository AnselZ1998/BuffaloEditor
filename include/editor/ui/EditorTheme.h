// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include <QIcon>
#include <QString>

class QApplication;
class QWidget;

namespace buffalo::editor
{
class EditorTheme
{
public:
    static bool Apply(QApplication& application);
    static bool ApplyPanelStyle(QWidget& panel, const QString& name);
    static QIcon Logo();
};
} // namespace buffalo::editor
