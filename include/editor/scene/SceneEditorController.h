// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include "core/EngineRuntime.h"
#include "editor/services/SelectionService.h"
#include "editor/services/EditorLog.h"
#include <QObject>
namespace buffalo::editor
{
struct CameraSettings
{
    Vector3 position;
    Vector3 target;
    Vector3 up;
    float fovDegrees = 60;
    float nearZ = 0.1f;
    float farZ = 1000;
    float aspect = 1.77f;
    bool autoAspect = true;
};
struct LightSettings
{
    bool enabled = true;
    Vector3 position;
    Vector3 direction;
    Vector3 color{1, 1, 1};
    float intensity = 1;
    LightOrbit orbit;
};
class SceneEditorController final : public QObject
{
    Q_OBJECT
public:
    SceneEditorController(EngineRuntime& runtime, SelectionService& selection, EditorLog& log, QObject* parent = nullptr);
    SceneManager& Scenes() const;
    Scene* ResolveScene(SelectionTarget target) const;
    Model* ResolveModel(SelectionTarget target) const;
    Camera* ResolveCamera(SelectionTarget target) const;
    ILight* ResolveLight(SelectionTarget target) const;
    bool IsValid(SelectionTarget target) const;
    void Select(SelectionTarget target);
    SceneId CreateScene();
    bool SetCurrentScene(SceneId id);
    void AddObject(SelectionKind kind);
    bool Rename(SelectionTarget target, const std::string& name);
    bool SetVisible(SelectionTarget target, bool visible);
    bool SetTransform(SelectionTarget target, Vector3 position, Vector3 rotation, Vector3 scale);
    bool SetMesh(SelectionTarget target, const std::string& path);
    bool SetTexture(SelectionTarget target, const std::string& path, bool normal);
    bool SetMaterial(SelectionTarget target, ShaderKind shader, Vector3 baseColor);
    bool SetCamera(SelectionTarget target, const CameraSettings& settings);
    bool SetActiveCamera(SelectionTarget target);
    bool SetLight(SelectionTarget target, const LightSettings& settings);
signals:
    void StructureChanged();
    void ObjectChanged();

private:
    void RebindIfCurrent(SceneId id);
    bool Fail(const std::string& message);
    EngineRuntime& runtime_;
    SelectionService& selection_;
    EditorLog& log_;
};
} // namespace buffalo::editor
