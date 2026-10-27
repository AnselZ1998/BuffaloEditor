// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/ui/EditorTheme.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QPalette>
#include <QWidget>

namespace buffalo::editor
{
bool EditorTheme::Apply(QApplication& application)
{
    application.setStyle("Fusion");
    // Palette also covers native/fallback controls; visual rules live in QSS.
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#292c30"));
    palette.setColor(QPalette::WindowText, QColor("#d3d7dc"));
    palette.setColor(QPalette::Base, QColor("#222529"));
    palette.setColor(QPalette::AlternateBase, QColor("#2d3035"));
    palette.setColor(QPalette::Text, QColor("#d3d7dc"));
    palette.setColor(QPalette::Button, QColor("#34383e"));
    palette.setColor(QPalette::ButtonText, QColor("#d3d7dc"));
    palette.setColor(QPalette::Highlight, QColor("#665039"));
    palette.setColor(QPalette::HighlightedText, QColor("#f2e6d6"));
    palette.setColor(QPalette::PlaceholderText, QColor("#858c95"));
    application.setPalette(palette);
    const QIcon logo = Logo();
    if (!logo.isNull())
    {
        application.setWindowIcon(logo);
    }
    QString combined;
    for (const auto* path : {"buffalo_dark.qss", "controls/inputs.qss", "panels/panels.qss", "panels/viewport/viewportbar.qss"})
    {
        QFile style(QString(":/buffalo/themes/") + path);
        if (!style.open(QIODevice::ReadOnly))
        {
            return false;
        }
        combined += QString::fromUtf8(style.readAll()) + "\n";
    }
    application.setStyleSheet(combined);
    return true;
}

bool EditorTheme::ApplyPanelStyle(QWidget& panel, const QString& name)
{
    QFile style(":/buffalo/themes/panels/" + name + ".qss");
    if (!style.open(QIODevice::ReadOnly))
    {
        return false;
    }
    panel.setStyleSheet(QString::fromUtf8(style.readAll()));
    return true;
}

QIcon EditorTheme::Logo()
{
    // The original PNG is embedded by CMake when supplied. A loose copy is
    // also accepted beside the executable without rebuilding the application.
    if (QFile::exists(":/buffalo/icons/buffalo_logo.png"))
    {
        return QIcon(":/buffalo/icons/buffalo_logo.png");
    }
    const QString loose = QDir(QCoreApplication::applicationDirPath()).filePath("resources/icons/buffalo_logo.png");
    return QFile::exists(loose) ? QIcon(loose) : QIcon();
}
} // namespace buffalo::editor
