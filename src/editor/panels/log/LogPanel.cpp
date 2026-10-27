// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/panels/log/LogPanel.h"

#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QStringList>
#include <QVBoxLayout>

namespace buffalo::editor
{
LogPanel::LogPanel(EditorLog& log, QWidget* parent)
    : QWidget(parent), log_(log)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 6, 8, 8);
    auto* tools = new QHBoxLayout;
    auto* clear = new QPushButton("Clear", this);
    clear->setObjectName("ClearLog");
    tools->addWidget(clear);
    tools->addStretch();
    auto* level = new QComboBox(this);
    level->setObjectName("LogLevel");
    level->addItems({"Trace", "Debug", "Info", "Warning", "Error"});
    level->setCurrentIndex(static_cast<int>(log.GetLevel()));
    level->setToolTip("Minimum level for newly recorded messages");
    tools->addWidget(level);
    connect(level, &QComboBox::currentIndexChanged, this, [this](int index)
    {
        log_.SetLevel(static_cast<LogLevel>(index));
    });
    tools->addWidget(new QLabel("Runtime / Render / Editor", this));
    layout->addLayout(tools);

    messages_ = new QPlainTextEdit(this);
    messages_->setObjectName("LogMessages");
    messages_->setReadOnly(true);
    messages_->setLineWrapMode(QPlainTextEdit::NoWrap);
    layout->addWidget(messages_);
    auto* command = new QLineEdit(this);
    command->setPlaceholderText("Console commands - coming later");
    command->setEnabled(false);
    layout->addWidget(command);
    connect(clear, &QPushButton::clicked, this, &LogPanel::Clear);
    Refresh();
}

void LogPanel::Refresh()
{
    if (revision_ == log_.Revision())
    {
        return;
    }
    revision_ = log_.Revision();
    QStringList lines;
    for (const auto& line : log_.Lines())
    {
        lines.append(QString::fromStdString(line));
    }
    const QString text = lines.join('\n');
    if (text == displayedText_)
    {
        return;
    }

    auto* scroll = messages_->verticalScrollBar();
    const int position = scroll->value();
    const bool atBottom = position >= scroll->maximum() - 2;
    displayedText_ = text;
    messages_->setPlainText(text);
    scroll->setValue(atBottom ? scroll->maximum() : position);
}

void LogPanel::Clear()
{
    log_.Clear();
    Refresh();
}
} // namespace buffalo::editor
