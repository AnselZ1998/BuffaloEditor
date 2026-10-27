// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/panels/hierarchy/HierarchyPanel.h"
#include "editor/scene/SceneEditorController.h"
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QToolButton>
#include <QMenu>
#include <QLabel>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <QTimer>
#include <set>
namespace buffalo::editor
{
HierarchyPanel::HierarchyPanel(SceneEditorController& controller, SelectionService& selection, QWidget* parent)
    : QWidget(parent), controller_(controller), selection_(selection)
{
    setMinimumWidth(210);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    auto* add = new QToolButton(this);
    add->setText("+  Add");
    add->setObjectName("AddSceneObject");
    add->setPopupMode(QToolButton::InstantPopup);
    auto* menu = new QMenu(add);
    menu->addAction("Scene", this, [this]()
    {
        controller_.CreateScene();
    });
    menu->addSeparator();
    menu->addAction("Model", this, [this]()
    {
        controller_.AddObject(SelectionKind::Model);
    });
    menu->addAction("Camera", this, [this]()
    {
        controller_.AddObject(SelectionKind::Camera);
    });
    menu->addAction("Directional Light", this, [this]()
    {
        controller_.AddObject(SelectionKind::Light);
    });
    add->setMenu(menu);
    layout->addWidget(add);
    tree_ = new QTreeWidget(this);
    tree_->setObjectName("SceneHierarchy");
    tree_->setHeaderLabels({"Scenes / Objects"});
    tree_->setAlternatingRowColors(true);
    tree_->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(tree_);
    auto* hint = new QLabel("Select a scene to switch. Model checkboxes control visibility.", this);
    hint->setObjectName("PanelHint");
    hint->setWordWrap(true);
    layout->addWidget(hint);
    connect(tree_, &QTreeWidget::itemSelectionChanged, this, [this]()
    {
        const auto selected = tree_->selectedItems();
        controller_.Select(selected.isEmpty() ? SelectionTarget{} : selected.front()->data(0, Qt::UserRole).value<SelectionTarget>());
    });
    connect(tree_, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* item, int)
    {
        const auto target = item->data(0, Qt::UserRole).value<SelectionTarget>();
        if (target.kind == SelectionKind::Model)
        {
            controller_.SetVisible(target, item->checkState(0) == Qt::Checked);
        }
    });
    connect(&selection_, &SelectionService::SelectionChanged, this, &HierarchyPanel::SyncSelection);
    connect(&controller_, &SceneEditorController::StructureChanged, this, [this]()
    {
        QTimer::singleShot(0, this, [this]()
        {
            Rebuild();
        });
    });
    connect(&controller_, &SceneEditorController::ObjectChanged, this, &HierarchyPanel::SyncSelection);
    Rebuild();
}
void HierarchyPanel::Rebuild()
{
    std::set<SceneId> expanded;
    for (int i = 0; i < tree_->topLevelItemCount(); ++i)
    {
        auto* item = tree_->topLevelItem(i);
        if (item->isExpanded())
        {
            expanded.insert(item->data(0, Qt::UserRole).value<SelectionTarget>().sceneId);
        }
    }
    const QSignalBlocker blocker(tree_);
    tree_->clear();
    auto* current = controller_.Scenes().GetCurrentScene();
    for (const auto& scene : controller_.Scenes().GetScenes())
    {
        const bool active = scene.get() == current;
        auto* root = new QTreeWidgetItem(tree_, {QString::fromStdString(scene->name) + (active ? "  [active]" : "")});
        root->setData(0, Qt::UserRole, QVariant::fromValue(SelectionTarget{SelectionKind::Scene, scene->Id(), 0}));
        QFont font = root->font(0);
        font.setBold(active);
        root->setFont(0, font);
        const auto add = [&](SelectionKind kind, ObjectId id, const std::string& name)
        {
            auto* item = new QTreeWidgetItem(root, {QString::fromStdString(name)});
            item->setData(0, Qt::UserRole, QVariant::fromValue(SelectionTarget{kind, scene->Id(), id}));
            return item;
        };
        for (const auto& model : scene->Models())
        {
            auto* item = add(SelectionKind::Model, model->Id(), model->name);
            item->setCheckState(0, model->visible ? Qt::Checked : Qt::Unchecked);
            item->setToolTip(0, QString::fromStdString(model->MeshPath()));
        }
        for (const auto& camera : scene->Cameras())
        {
            add(SelectionKind::Camera, camera->Id(), camera->name + (camera.get() == scene->getCurrentCamera() ? " [camera]" : ""));
        }
        for (const auto& light : scene->Lights())
        {
            add(SelectionKind::Light, light->Id(), light->name);
        }
        root->setExpanded(expanded.count(scene->Id()) != 0 || active);
    }
    SyncSelection();
}
void HierarchyPanel::SyncSelection()
{
    const QSignalBlocker blocker(tree_);
    tree_->clearSelection();
    for (QTreeWidgetItemIterator it(tree_); *it; ++it)
    {
        const auto target = (*it)->data(0, Qt::UserRole).value<SelectionTarget>();
        if (auto* model = controller_.ResolveModel(target))
        {
            (*it)->setCheckState(0, model->visible ? Qt::Checked : Qt::Unchecked);
        }
        if (target == selection_.Selected())
        {
            (*it)->setSelected(true);
            if ((*it)->parent())
            {
                (*it)->parent()->setExpanded(true);
            }
        }
    }
}
} // namespace buffalo::editor
