// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "scene/Scene.h"
#include <filesystem>
#ifndef BUFFALO_DEFAULT_MODEL_PATH
#define BUFFALO_DEFAULT_MODEL_PATH "model/toy.obj"
#endif
Scene::Scene(const std::string& sceneName, bool loadDefaultModel): name(sceneName)
{
    AddCamera();
    AddLight();
    if (loadDefaultModel)
    {
        auto& model = AddModel(BUFFALO_DEFAULT_MODEL_PATH);
        const auto texture = std::filesystem::path(BUFFALO_DEFAULT_MODEL_PATH).parent_path() / "texture/toy/ToyShader_BaseColor.png";
        if (std::filesystem::exists(texture))
        {
            model.GetMaterial()->LoadTexture(texture.string(), false);
        }
    }
}
Model& Scene::AddModel(const std::string& path)
{
    auto model = std::make_unique<Model>(path);
    model->name = "Model " + std::to_string(models_.size() + 1);
    models_.push_back(std::move(model));
    return *models_.back();
}
Camera& Scene::AddCamera()
{
    auto camera = std::make_unique<Camera>();
    camera->name = "Camera " + std::to_string(cameras_.size() + 1);
    if (!activeCamera_)
    {
        activeCamera_ = camera->Id();
    }
    cameras_.push_back(std::move(camera));
    return *cameras_.back();
}
ILight& Scene::AddLight()
{
    auto light = std::make_unique<DirectionalLight>();
    light->name += " " + std::to_string(lights_.size() + 1);
    lights_.push_back(std::move(light));
    return *lights_.back();
}
Model* Scene::FindModel(ObjectId id) const
{
    for (const auto& model : models_)
    {
        if (model->Id() == id)
        {
            return model.get();
        }
    }
    return nullptr;
}
Camera* Scene::FindCamera(ObjectId id) const
{
    for (const auto& camera : cameras_)
    {
        if (camera->Id() == id)
        {
            return camera.get();
        }
    }
    return nullptr;
}
ILight* Scene::FindLight(ObjectId id) const
{
    for (const auto& light : lights_)
    {
        if (light->Id() == id)
        {
            return light.get();
        }
    }
    return nullptr;
}
bool Scene::SetActiveCamera(ObjectId id)
{
    if (!FindCamera(id))
    {
        return false;
    }
    activeCamera_ = id;
    return true;
}
Camera* Scene::getCurrentCamera()
{
    return FindCamera(activeCamera_);
}
std::queue<Model*>* Scene::GetVisibleModels()
{
    visibleModels_ = {};
    for (const auto& model : models_)
    {
        if (model->visible && model->MeshValid())
        {
            visibleModels_.push(model.get());
        }
    }
    return &visibleModels_;
}
Bounds3 Scene::GetWorldBounds() const
{
    Bounds3 bounds;
    for (const auto& model : models_)
    {
        if (!model->visible || !model->MeshValid())
        {
            continue;
        }
        for (const auto& vertex : model->GetMesh()->vertices)
        {
            bounds.Expand((model->GetModelMatrix() * Vector4::FromPoint(vertex)).XYZ());
        }
    }
    return bounds;
}
bool Scene::CheckIfEmpty()
{
    return !GetVisibleModels()->empty();
}
void Scene::UpdateLights(double elapsedSeconds)
{
    for (const auto& light : lights_)
    {
        light->UpdateOrbit(elapsedSeconds);
    }
}

std::vector<DirectionalLightParameters> Scene::Lighting() const
{
    std::vector<DirectionalLightParameters> result;
    for (const auto& light : lights_)
    {
        if (light->enabled && light->Kind() == LightKind::Directional)
        {
            result.push_back({light->direction.Normalized() * -1.0f, light->color * light->intensity});
        }
    }
    return result;
}
