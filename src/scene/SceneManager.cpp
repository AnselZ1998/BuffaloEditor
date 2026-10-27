// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "scene/SceneManager.h"
bool SceneManager::StartUp()
{
    if (scenes_.empty())
    {
        scenes_.push_back(std::make_unique<Scene>("Default Scene"));
        current_ = scenes_.back()->Id();
    }
    return true;
}
void SceneManager::ShutDown()
{
    current_ = 0;
    scenes_.clear();
}
Scene& SceneManager::CreateScene(const std::string& name)
{
    scenes_.push_back(std::make_unique<Scene>(name, false));
    if (!current_)
    {
        current_ = scenes_.back()->Id();
    }
    return *scenes_.back();
}
bool SceneManager::SetCurrentScene(SceneId id)
{
    if (!FindScene(id))
    {
        return false;
    }
    current_ = id;
    return true;
}
Scene* SceneManager::GetCurrentScene() const
{
    return FindScene(current_);
}
Scene* SceneManager::FindScene(SceneId id) const
{
    for (const auto& scene : scenes_)
    {
        if (scene->Id() == id)
        {
            return scene.get();
        }
    }
    return nullptr;
}
