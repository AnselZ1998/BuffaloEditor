// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/panels/inspector/InspectorPanel.h"
#include "editor/scene/SceneEditorController.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <array>
#include <functional>
namespace buffalo::editor
{
namespace
{
struct VectorEdit
{
    std::array<QDoubleSpinBox*, 3> values;
    Vector3 Read() const
    {
        return {static_cast<float>(values[0]->value()), static_cast<float>(values[1]->value()), static_cast<float>(values[2]->value())};
    }
};
QDoubleSpinBox* Number(QFormLayout* form, const QString& name, double value, double minimum = -1000000, double maximum = 1000000)
{
    auto* edit = new QDoubleSpinBox;
    edit->setObjectName(name);
    edit->setRange(minimum, maximum);
    edit->setDecimals(4);
    edit->setSingleStep(0.1);
    edit->setValue(value);
    edit->setKeyboardTracking(false);
    form->addRow(name, edit);
    return edit;
}
VectorEdit Vector(QFormLayout* form, const QString& name, Vector3 value, double minimum = -1000000, double maximum = 1000000)
{
    auto* row = new QWidget;
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);
    VectorEdit result;
    for (int i = 0; i < 3; ++i)
    {
        auto* edit = new QDoubleSpinBox;
        edit->setObjectName(name + QString::number(i));
        edit->setRange(minimum, maximum);
        edit->setDecimals(3);
        edit->setSingleStep(0.1);
        edit->setValue(value.v[i]);
        edit->setPrefix(QString("XYZ").mid(i, 1) + " ");
        edit->setKeyboardTracking(false);
        edit->setMinimumWidth(64);
        layout->addWidget(edit);
        result.values[i] = edit;
    }
    form->addRow(name, row);
    return result;
}
QFormLayout* Group(QVBoxLayout* layout, const QString& name)
{
    auto* group = new QGroupBox(name);
    group->setCheckable(true);
    group->setChecked(true);
    auto* body = new QWidget(group);
    auto* form = new QFormLayout(body);
    form->setContentsMargins(6, 2, 6, 2);
    form->setSpacing(6);
    form->setRowWrapPolicy(QFormLayout::WrapLongRows);
    auto* outer = new QVBoxLayout(group);
    outer->setContentsMargins(6, 12, 6, 6);
    outer->addWidget(body);
    QObject::connect(group, &QGroupBox::toggled, body, &QWidget::setVisible);
    layout->addWidget(group);
    return form;
}
void Apply(QFormLayout* form, const QString& name, std::function<bool()> action)
{
    auto* button = new QPushButton(name);
    button->setObjectName(name);
    auto* status = new QLabel;
    status->setWordWrap(true);
    status->setObjectName("PanelHint");
    form->addRow(button);
    form->addRow(status);
    status->hide();
    QObject::connect(button, &QPushButton::clicked, button, [action, status]()
    {
        status->setText(action() ? "Applied" : "Not applied. Check values and Log.");
        status->show();
    });
}
void Path(QFormLayout* form, const QString& name, const std::string& path, const QString& filter, std::function<bool(const std::string&)> action)
{
    auto* row = new QWidget;
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* text = new QLineEdit(QString::fromStdString(path));
    text->setObjectName(name);
    text->setPlaceholderText("Empty clears the resource");
    auto* browse = new QPushButton("...");
    browse->setMaximumWidth(28);
    layout->addWidget(text);
    layout->addWidget(browse);
    form->addRow(name, row);
    QObject::connect(browse, &QPushButton::clicked, row, [text, filter, row]()
    {
        const QString file = QFileDialog::getOpenFileName(row, "Select asset", text->text(), filter);
        if (!file.isEmpty())
        {
            text->setText(file);
        }
    });
    Apply(form, "Apply " + name, [text, action]()
    {
        return action(text->text().toStdString());
    });
}
} // namespace
InspectorPanel::InspectorPanel(SceneEditorController& controller, SelectionService& selection, QWidget* parent)
    : QWidget(parent), controller_(controller), selection_(selection)
{
    setMinimumWidth(360);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    title_ = new QLabel(this);
    title_->setObjectName("SelectionTitle");
    layout->addWidget(title_);
    scroll_ = new QScrollArea(this);
    scroll_->setWidgetResizable(true);
    scroll_->setFrameShape(QFrame::NoFrame);
    layout->addWidget(scroll_);
    connect(&selection_, &SelectionService::SelectionChanged, this, &InspectorPanel::Rebuild);
    connect(&controller_, &SceneEditorController::StructureChanged, this, [this]()
    {
        QTimer::singleShot(0, this, [this]()
        {
            Rebuild();
        });
    });
    Rebuild();
}
void InspectorPanel::Rebuild()
{
    // Destroy the previous form later: selection can change inside one of its callbacks.
    if (auto* old = scroll_->takeWidget())
    {
        old->hide();
        old->deleteLater();
    }
    auto* body = new QWidget;
    auto* layout = new QVBoxLayout(body);
    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    scroll_->setWidget(body);
    const SelectionTarget target = selection_.Selected();
    auto* scene = controller_.ResolveScene(target);
    if (!scene || target.kind == SelectionKind::None || !controller_.IsValid(target))
    {
        title_->setText("Nothing selected");
        layout->addWidget(new QLabel("Select an object in the Hierarchy."));
        layout->addStretch();
        return;
    }
    std::string name = scene->name;
    if (auto* model = controller_.ResolveModel(target))
    {
        name = model->name;
    }
    if (auto* camera = controller_.ResolveCamera(target))
    {
        name = camera->name;
    }
    if (auto* light = controller_.ResolveLight(target))
    {
        name = light->name;
    }
    title_->setText(QString::fromStdString(name));
    auto* identity = Group(layout, "Object");
    auto* nameEdit = new QLineEdit(QString::fromStdString(name));
    nameEdit->setObjectName("ObjectName");
    identity->addRow("Name", nameEdit);
    Apply(identity, "Rename", [this, target, nameEdit]()
    {
        return controller_.Rename(target, nameEdit->text().toStdString());
    });
    if (auto* model = controller_.ResolveModel(target))
    {
        auto* visible = new QCheckBox("Visible");
        visible->setObjectName("ModelVisible");
        visible->setChecked(model->visible);
        identity->addRow(visible);
        connect(visible, &QCheckBox::toggled, body, [this, target](bool checked)
        {
            controller_.SetVisible(target, checked);
        });
        connect(&controller_, &SceneEditorController::ObjectChanged, body, [this, target, visible]()
        {
            if (auto* current = controller_.ResolveModel(target))
            {
                const QSignalBlocker blocker(visible);
                visible->setChecked(current->visible);
            }
        });
        auto* transform = Group(layout, "Transform");
        const auto position = Vector(transform, "Position", model->Position());
        const auto rotation = Vector(transform, "Rotation (deg)", model->RotationDegrees());
        const auto scale = Vector(transform, "Scale", model->Scale(), 0.001, 100000);
        Apply(transform, "Apply Transform", [this, target, position, rotation, scale]()
        {
            return controller_.SetTransform(target, position.Read(), rotation.Read(), scale.Read());
        });
        auto* mesh = Group(layout, "Mesh");
        Path(mesh, "Mesh Path", model->MeshPath(), "Wavefront OBJ (*.obj)", [this, target](const std::string& path)
        {
            return controller_.SetMesh(target, path);
        });
        auto* material = Group(layout, "Material");
        const auto* value = model->GetMaterial();
        auto* shader = new QComboBox;
        shader->setObjectName("Shader");
        shader->addItems({"BlinnPhong (diffuse)", "Unlit", "Normal debug"});
        shader->setCurrentIndex(static_cast<int>(value->GetShaderKind()));
        material->addRow("Shader", shader);
        const auto color = Vector(material, "Base Color", value->baseColor, 0, 1);
        Apply(material, "Apply Material", [this, target, shader, color]()
        {
            return controller_.SetMaterial(target, static_cast<ShaderKind>(shader->currentIndex()), color.Read());
        });
        Path(material, "BaseColor Texture", value->TexturePath(false), "Images (*.png *.jpg *.jpeg *.tga *.bmp)", [this, target](const std::string& path)
        {
            return controller_.SetTexture(target, path, false);
        });
        Path(material, "Normal Texture", value->TexturePath(true), "Images (*.png *.jpg *.jpeg *.tga *.bmp)", [this, target](const std::string& path)
        {
            return controller_.SetTexture(target, path, true);
        });
    }
    else if (auto* camera = controller_.ResolveCamera(target))
    {
        auto* form = Group(layout, "Camera");
        const auto position = Vector(form, "Position", camera->position);
        const auto lookAt = Vector(form, "Target", camera->target);
        const auto up = Vector(form, "Up", camera->up);
        auto* fov = Number(form, "FOV (deg)", camera->fovYRadians * 57.29577951, 1, 179);
        auto* nearZ = Number(form, "Near", camera->nearZ, 0.0001, 1000000);
        auto* farZ = Number(form, "Far", camera->farZ, 0.001, 10000000);
        auto* aspect = Number(form, "Aspect", camera->aspect, 0.001, 100);
        auto* autoAspect = new QCheckBox("Match viewport aspect");
        autoAspect->setChecked(camera->autoAspect);
        form->addRow(autoAspect);
        Apply(form, "Apply Camera", [=]()
        {
            return controller_.SetCamera(target, {position.Read(), lookAt.Read(), up.Read(), static_cast<float>(fov->value()), static_cast<float>(nearZ->value()),
                                                     static_cast<float>(farZ->value()), static_cast<float>(aspect->value()), autoAspect->isChecked()});
        });
        Apply(form, "Use Camera", [this, target]()
        {
            return controller_.SetActiveCamera(target);
        });
    }
    else if (auto* light = controller_.ResolveLight(target))
    {
        auto* form = Group(layout, "Directional Light");
        auto* enabled = new QCheckBox("Enabled");
        enabled->setChecked(light->enabled);
        form->addRow(enabled);
        const auto position = Vector(form, "Position", light->position);
        const auto direction = Vector(form, "Direction", light->direction);
        const auto color = Vector(form, "Color", light->color, 0, 1);
        auto* intensity = Number(form, "Intensity", light->intensity, 0, 100);
        auto* orbitForm = Group(layout, "Orbit debug");
        auto* orbit = new QCheckBox("Orbit around target");
        orbit->setObjectName("LightOrbit");
        orbit->setChecked(light->orbit.enabled);
        orbitForm->addRow(orbit);
        const auto center = Vector(orbitForm, "Orbit Target", light->orbit.target);
        auto* radius = Number(orbitForm, "Radius", light->orbit.radius, 0.001, 100000);
        auto* height = Number(orbitForm, "Height", light->orbit.height);
        auto* speed = Number(orbitForm, "Speed (deg/s)", light->orbit.speedDegrees, -3600, 3600);
        auto* angle = Number(orbitForm, "Phase (deg)", light->orbit.angleDegrees, -360, 360);
        connect(orbit, &QCheckBox::toggled, body, [this, target, position, direction, angle](bool checked)
        {
            auto* current = controller_.ResolveLight(target);
            if (!current)
            {
                return;
            }
            LightSettings settings{current->enabled, current->position, current->direction, current->color, current->intensity, current->orbit};
            settings.orbit.enabled = checked;
            controller_.SetLight(target, settings);
            for (int i = 0; i < 3; ++i)
            {
                position.values[i]->setValue(current->position.v[i]);
                direction.values[i]->setValue(current->direction.v[i]);
            }
            angle->setValue(current->orbit.angleDegrees);
        });
        Apply(orbitForm, "Apply Light", [=]()
        {
            LightSettings settings;
            settings.enabled = enabled->isChecked();
            settings.position = position.Read();
            settings.direction = direction.Read();
            settings.color = color.Read();
            settings.intensity = static_cast<float>(intensity->value());
            settings.orbit = {orbit->isChecked(), center.Read(), static_cast<float>(radius->value()), static_cast<float>(height->value()),
                static_cast<float>(speed->value()), static_cast<float>(angle->value())};
            return controller_.SetLight(target, settings);
        });
        auto* live = new QLabel;
        live->setWordWrap(true);
        live->setObjectName("PanelHint");
        orbitForm->addRow(live);
        auto* timer = new QTimer(body);
        connect(timer, &QTimer::timeout, body, [this, target, live]()
        {
            if (auto* current = controller_.ResolveLight(target))
            {
                live->setText(QString("Live position: %1, %2, %3\nDirection: %4, %5, %6")
                        .arg(current->position.x, 0, 'f', 2)
                        .arg(current->position.y, 0, 'f', 2)
                        .arg(current->position.z, 0, 'f', 2)
                        .arg(current->direction.x, 0, 'f', 2)
                        .arg(current->direction.y, 0, 'f', 2)
                        .arg(current->direction.z, 0, 'f', 2));
            }
        });
        timer->start(100);
        auto* hint = new QLabel("Orbit uses world Y as its axis and aims at the target. Directional light position is for debugging; direction controls illumination.");
        hint->setWordWrap(true);
        hint->setObjectName("PanelHint");
        orbitForm->addRow(hint);
    }
    else
    {
        auto* label = new QLabel("Add objects from the Hierarchy. New scenes start with a camera and a directional light.");
        label->setWordWrap(true);
        layout->addWidget(label);
    }
    layout->addStretch();
}
} // namespace buffalo::editor
