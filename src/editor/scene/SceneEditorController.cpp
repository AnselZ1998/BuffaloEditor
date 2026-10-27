// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/scene/SceneEditorController.h"
#include <cmath>
#include <exception>
namespace buffalo::editor
{
namespace
{
bool Finite(Vector3 v)
{
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
bool ColorValid(Vector3 v)
{
    return Finite(v) && v.x >= 0 && v.y >= 0 && v.z >= 0 && v.x <= 1 && v.y <= 1 && v.z <= 1;
}
} // namespace
SceneEditorController::SceneEditorController(EngineRuntime& runtime, SelectionService& selection, EditorLog& log, QObject* parent)
    : QObject(parent), runtime_(runtime), selection_(selection), log_(log)
{
}
SceneManager& SceneEditorController::Scenes() const
{
    return runtime_.GetSceneManager();
}
Scene* SceneEditorController::ResolveScene(SelectionTarget target) const
{
    return Scenes().FindScene(target.sceneId);
}
Model* SceneEditorController::ResolveModel(SelectionTarget target) const
{
    auto* scene = ResolveScene(target);
    return scene && target.kind == SelectionKind::Model ? scene->FindModel(target.objectId) : nullptr;
}
Camera* SceneEditorController::ResolveCamera(SelectionTarget target) const
{
    auto* scene = ResolveScene(target);
    return scene && target.kind == SelectionKind::Camera ? scene->FindCamera(target.objectId) : nullptr;
}
ILight* SceneEditorController::ResolveLight(SelectionTarget target) const
{
    auto* scene = ResolveScene(target);
    return scene && target.kind == SelectionKind::Light ? scene->FindLight(target.objectId) : nullptr;
}
bool SceneEditorController::IsValid(SelectionTarget target) const
{
    switch (target.kind)
    {
    case SelectionKind::None:
        return target.sceneId == 0 && target.objectId == 0;
    case SelectionKind::Scene:
        return ResolveScene(target) && target.objectId == 0;
    case SelectionKind::Model:
        return ResolveModel(target) != nullptr;
    case SelectionKind::Camera:
        return ResolveCamera(target) != nullptr;
    case SelectionKind::Light:
        return ResolveLight(target) != nullptr;
    }
    return false;
}
void SceneEditorController::Select(SelectionTarget target)
{
    if (!IsValid(target))
    {
        selection_.Clear();
        return;
    }
    if (target.kind != SelectionKind::None)
    {
        SetCurrentScene(target.sceneId);
    }
    selection_.Select(target);
}
SceneId SceneEditorController::CreateScene()
{
    auto& scene = Scenes().CreateScene("Scene " + std::to_string(Scenes().GetScenes().size() + 1));
    const auto id = scene.Id();
    SetCurrentScene(id);
    selection_.Select({SelectionKind::Scene, id, 0});
    emit StructureChanged();
    log_.Info("Created Scene: " + scene.name);
    return id;
}
bool SceneEditorController::SetCurrentScene(SceneId id)
{
    if (!Scenes().FindScene(id))
    {
        return Fail("Scene no longer exists.");
    }
    if (Scenes().GetCurrentScene() && Scenes().GetCurrentScene()->Id() == id)
    {
        return true;
    }
    Scenes().SetCurrentScene(id);
    runtime_.RebindSimulation();
    selection_.Clear();
    emit StructureChanged();
    log_.Info("Active Scene: " + Scenes().GetCurrentScene()->name);
    return true;
}
void SceneEditorController::AddObject(SelectionKind kind)
{
    auto* scene = Scenes().GetCurrentScene();
    if (!scene)
    {
        return;
    }
    ObjectId id = 0;
    switch (kind)
    {
    case SelectionKind::Model:
        id = scene->AddModel().Id();
        RebindIfCurrent(scene->Id());
        break;
    case SelectionKind::Camera:
        id = scene->AddCamera().Id();
        break;
    case SelectionKind::Light:
        id = scene->AddLight().Id();
        break;
    default:
        return;
    }
    emit StructureChanged();
    selection_.Select({kind, scene->Id(), id});
    log_.Info("Created object in " + scene->name);
}
bool SceneEditorController::Rename(SelectionTarget target, const std::string& name)
{
    if (name.empty())
    {
        return Fail("Name cannot be empty.");
    }
    if (auto* model = ResolveModel(target))
    {
        model->name = name;
    }
    else if (auto* camera = ResolveCamera(target))
    {
        camera->name = name;
    }
    else if (auto* light = ResolveLight(target))
    {
        light->name = name;
    }
    else if (target.kind == SelectionKind::Scene && ResolveScene(target))
    {
        ResolveScene(target)->name = name;
    }
    else
    {
        return false;
    }
    emit StructureChanged();
    return true;
}
void SceneEditorController::RebindIfCurrent(SceneId id)
{
    if (Scenes().GetCurrentScene() && Scenes().GetCurrentScene()->Id() == id)
    {
        runtime_.RebindSimulation();
    }
}
bool SceneEditorController::SetVisible(SelectionTarget target, bool visible)
{
    auto* model = ResolveModel(target);
    if (!model)
    {
        return false;
    }
    model->visible = visible;
    RebindIfCurrent(target.sceneId);
    emit ObjectChanged();
    return true;
}
bool SceneEditorController::SetTransform(SelectionTarget target, Vector3 position, Vector3 rotation, Vector3 scale)
{
    auto* model = ResolveModel(target);
    if (!model)
    {
        return false;
    }
    if (!Finite(position) || !Finite(rotation) || !Finite(scale) || scale.x < 0.001f || scale.y < 0.001f || scale.z < 0.001f)
    {
        return Fail("Transform needs finite values and positive scale.");
    }
    model->SetTransform(position, rotation, scale);
    RebindIfCurrent(target.sceneId);
    emit ObjectChanged();
    return true;
}
bool SceneEditorController::SetMesh(SelectionTarget target, const std::string& path)
{
    auto* model = ResolveModel(target);
    if (!model)
    {
        return false;
    }
    if (!model->LoadMesh(path))
    {
        return Fail("Could not load mesh; previous mesh retained: " + path);
    }
    RebindIfCurrent(target.sceneId);
    log_.Info("Mesh loaded: " + path);
    emit ObjectChanged();
    return true;
}
bool SceneEditorController::SetTexture(SelectionTarget target, const std::string& path, bool normal)
{
    auto* model = ResolveModel(target);
    if (!model)
    {
        return false;
    }
    try
    {
        if (!model->GetMaterial()->LoadTexture(path, normal))
        {
            return Fail("Could not load texture; previous texture retained: " + path);
        }
    }
    catch (const std::exception& error)
    {
        return Fail(error.what());
    }
    log_.Info("Texture updated: " + path);
    emit ObjectChanged();
    return true;
}
bool SceneEditorController::SetMaterial(SelectionTarget target, ShaderKind shader, Vector3 baseColor)
{
    auto* model = ResolveModel(target);
    if (!model || !ColorValid(baseColor))
    {
        return false;
    }
    if (shader != ShaderKind::BlinnPhong && shader != ShaderKind::Unlit && shader != ShaderKind::Normal)
    {
        return false;
    }
    model->GetMaterial()->SetShader(shader);
    model->GetMaterial()->baseColor = baseColor;
    emit ObjectChanged();
    return true;
}
bool SceneEditorController::SetCamera(SelectionTarget target, const CameraSettings& s)
{
    auto* camera = ResolveCamera(target);
    if (!camera)
    {
        return false;
    }
    const Vector3 forward = s.target - s.position;
    if (!Finite(s.position) || !Finite(s.target) || !Finite(s.up) || forward.Length() < 1e-5f ||
        Vector3::Cross(forward.Normalized(), s.up.Normalized()).Length() < 1e-5f ||
        !std::isfinite(s.fovDegrees) || s.fovDegrees < 1 || s.fovDegrees > 179 ||
        !std::isfinite(s.nearZ) || !std::isfinite(s.farZ) || s.nearZ <= 0 || s.farZ <= s.nearZ ||
        !std::isfinite(s.aspect) || s.aspect <= 0)
    {
        return Fail("Invalid camera: check target, up vector, FOV and clipping planes.");
    }
    camera->position = s.position;
    camera->target = s.target;
    camera->up = s.up.Normalized();
    camera->fovYRadians = s.fovDegrees * 0.01745329252f;
    camera->nearZ = s.nearZ;
    camera->farZ = s.farZ;
    camera->aspect = s.aspect;
    camera->autoAspect = s.autoAspect;
    camera->UpdateMatrices();
    emit ObjectChanged();
    return true;
}
bool SceneEditorController::SetActiveCamera(SelectionTarget target)
{
    if (!ResolveCamera(target))
    {
        return false;
    }
    ResolveScene(target)->SetActiveCamera(target.objectId);
    emit StructureChanged();
    return true;
}
bool SceneEditorController::SetLight(SelectionTarget target, const LightSettings& s)
{
    auto* light = ResolveLight(target);
    if (!light)
    {
        return false;
    }
    if (!Finite(s.position) || !Finite(s.direction) || s.direction.Length() < 1e-5f || !ColorValid(s.color) ||
        !std::isfinite(s.intensity) || s.intensity < 0 || !Finite(s.orbit.target) ||
        !std::isfinite(s.orbit.radius) || s.orbit.radius < 0.001f || !std::isfinite(s.orbit.height) ||
        !std::isfinite(s.orbit.speedDegrees) || !std::isfinite(s.orbit.angleDegrees))
    {
        return Fail("Invalid light or orbit parameters.");
    }
    light->enabled = s.enabled;
    light->position = s.position;
    light->direction = s.direction.Normalized();
    light->color = s.color;
    light->intensity = s.intensity;
    light->orbit = s.orbit;
    light->UpdateOrbit(0);
    emit ObjectChanged();
    return true;
}
bool SceneEditorController::Fail(const std::string& message)
{
    log_.Warning(message);
    return false;
}
} // namespace buffalo::editor
