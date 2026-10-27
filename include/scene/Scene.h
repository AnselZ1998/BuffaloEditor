// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include "assets/model/Model.h"
#include "scene/Camera.h"
#include "scene/light/DirectionalLight.h"
#include "scene/ObjectId.h"
#include "math/BoundingVolume.h"
#include <memory>
#include <queue>
#include <string>
#include <vector>
class Scene
{
public:
    explicit Scene(const std::string& name, bool loadDefaultModel = true);
    SceneId Id() const
    {
        return id_;
    }
    std::string name;
    Model& AddModel(const std::string& path = {});
    Camera& AddCamera();
    ILight& AddLight();
    Model* FindModel(ObjectId id) const;
    Camera* FindCamera(ObjectId id) const;
    ILight* FindLight(ObjectId id) const;
    const std::vector<std::unique_ptr<Model>>& Models() const
    {
        return models_;
    }
    const std::vector<std::unique_ptr<Camera>>& Cameras() const
    {
        return cameras_;
    }
    const std::vector<std::unique_ptr<ILight>>& Lights() const
    {
        return lights_;
    }
    bool SetActiveCamera(ObjectId id);
    Camera* getCurrentCamera();
    std::queue<Model*>* GetVisibleModels();
    Bounds3 GetWorldBounds() const;
    // Legacy name: true means a renderable model exists.
    bool CheckIfEmpty();
    void UpdateLights(double elapsedSeconds);
    std::vector<DirectionalLightParameters> Lighting() const;

private:
    const SceneId id_ = NewObjectId();
    ObjectId activeCamera_ = 0;
    std::vector<std::unique_ptr<Model>> models_;
    std::vector<std::unique_ptr<Camera>> cameras_;
    std::vector<std::unique_ptr<ILight>> lights_;
    std::queue<Model*> visibleModels_;
};
