// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include "scene/Scene.h"
class SceneManager
{
public:
    bool StartUp();
    void ShutDown();
    Scene& CreateScene(const std::string& name);
    bool SetCurrentScene(SceneId id);
    Scene* GetCurrentScene() const;
    Scene* FindScene(SceneId id) const;
    const std::vector<std::unique_ptr<Scene>>& GetScenes() const
    {
        return scenes_;
    }

private:
    SceneId current_ = 0;
    std::vector<std::unique_ptr<Scene>> scenes_;
};
