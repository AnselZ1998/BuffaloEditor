// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include "render/RenderSystem.h"
#include "scene/SceneManager.h"
#include "core/SceneSimulationBinding.h"

namespace buffalo
{
// Runtime composition root. SceneManager keeps its existing scene ownership.
class EngineRuntime
{
public:
    EngineRuntime();
    ~EngineRuntime();
    EngineRuntime(const EngineRuntime&) = delete;
    EngineRuntime& operator=(const EngineRuntime&) = delete;
    bool StartUp();
    void SetLogger(ILogger* logger);
    void RebindSimulation();
    void ShutDown();
    bool IsStarted() const;
    SceneManager& GetSceneManager();
    RenderSystem& GetRenderSystem();
    const SimulationSystem& GetSimulationSystem() const;
    void Update(double elapsedSeconds);
    void PlaySimulation();
    void PauseSimulation();
    void StepSimulation();
    void ResetSimulation();
    float SimulationGroundHeight() const
    {
        return simulationBinding_.GroundHeight();
    }

private:
    SceneManager sceneManager_{};
    RenderSystem renderSystem_;
    SimulationSystem simulation_;
    SceneSimulationBinding simulationBinding_;
    bool started_ = false;
    SceneId simulationScene_ = 0;
    ILogger* logger_ = nullptr;
};
} // namespace buffalo
