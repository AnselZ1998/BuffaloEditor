// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/panels/hierarchy/HierarchyPanel.h"
#include "editor/services/SelectionService.h"

#include <QLabel>
#include <QSignalBlocker>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace buffalo::editor
{
HierarchyPanel::HierarchyPanel(SelectionService& selection, bool sceneReady, QWidget* parent)
    : QWidget(parent)
{
    setMinimumWidth(170);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    auto* tree = new QTreeWidget(this);
    tree->setObjectName("SceneHierarchy");
    tree->setHeaderLabels({"Scene objects"});
    tree->setRootIsDecorated(false);
    layout->addWidget(tree);
    QTreeWidgetItem* sceneItem = nullptr;
    if (sceneReady)
    {
        sceneItem = new QTreeWidgetItem(tree, {"Scene"});
        sceneItem->setData(0, Qt::UserRole, "scene");
    }
    auto* hint = new QLabel(sceneReady ? "Scene root only. Object hierarchy is not connected yet." : "No scene loaded. Check Log.", this);
    hint->setObjectName("PanelHint");
    hint->setWordWrap(true);
    layout->addWidget(hint);
    connect(tree, &QTreeWidget::itemSelectionChanged, this, [&selection, tree]()
    {
        const auto items = tree->selectedItems();
        selection.Select(items.isEmpty() ? QString() : items.front()->data(0, Qt::UserRole).toString());
    });
    const auto refresh = [&selection, tree, sceneItem]()
    {
        const QSignalBlocker blocker(tree);
        tree->clearSelection();
        if (sceneItem && selection.SelectedId() == "scene")
        {
            sceneItem->setSelected(true);
        }
    };
    connect(&selection, &SelectionService::SelectionChanged, this, refresh);
    refresh();
}
} // namespace buffalo::editor
