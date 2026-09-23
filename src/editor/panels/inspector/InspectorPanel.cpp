// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/panels/inspector/InspectorPanel.h"
#include "editor/services/SelectionService.h"

#include <QLabel>
#include <QVBoxLayout>

namespace buffalo::editor
{
InspectorPanel::InspectorPanel(SelectionService& selection, QWidget* parent)
    : QWidget(parent)
{
    setMinimumWidth(200);
    auto* layout = new QVBoxLayout(this);
    auto* title = new QLabel(this);
    title->setObjectName("SelectionTitle");
    title->setAlignment(Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(title);
    auto* hint = new QLabel("Property editing will be connected in a later version.", this);
    hint->setObjectName("PanelHint");
    hint->setWordWrap(true);
    hint->setAlignment(Qt::AlignCenter);
    layout->addWidget(hint);
    layout->addStretch();
    const auto refresh = [&selection, title]()
    {
        title->setText(selection.SelectedId().isEmpty() ? "Nothing selected" : "Selected: " + selection.SelectedId());
    };
    connect(&selection, &SelectionService::SelectionChanged, this, refresh);
    refresh();
}
} // namespace buffalo::editor
