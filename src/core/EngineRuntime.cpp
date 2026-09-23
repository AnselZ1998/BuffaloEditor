// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "core/EngineRuntime.h"
#include "render/software/SoftwareRenderBackend.h"
#include "simulation/rigidbody/cpu/CpuRigidBodySolver.h"
#include <memory>

namespace buffalo
{
EngineRuntime::EngineRuntime()
    : simulation_(std::make_unique<CpuRigidBodySolver>())
{
}

EngineRuntime::~EngineRuntime()
{
    ShutDown();
}

bool EngineRuntime::StartUp()
{
    if (started_)
    {
        return true;
    }
    try
    {
        // Runtime startup does not require a Model; SceneManager accepts an empty Scene.
        if (!sceneManager_.StartUp())
        {
            ShutDown();
            return false;
        }
        renderSystem_.AddBackend(std::make_unique<SoftwareRenderBackend>());
        simulationBinding_.Bind(*sceneManager_.GetCurrentScene(), simulation_);

        // TODO: add other render here
        // renderSystem_.AddBackend(std::make_unique<OpenGLRenderBackend>());

        started_ = true;
        return true;
    }
    catch (...)
    {
        ShutDown();
        throw;
    }
}

void EngineRuntime::ShutDown()
{
    simulation_.Pause();
    simulationBinding_.Clear();
    simulation_.Initialize({}, {});
    // Backends may borrow the scene camera; release them before the scene.
    renderSystem_.ShutDown();
    // SceneManager makes repeated shutdown safe and owns Scene cleanup.
    sceneManager_.ShutDown();
    started_ = false;
}

bool EngineRuntime::IsStarted() const
{
    return started_;
}

SceneManager& EngineRuntime::GetSceneManager()
{
    return sceneManager_;
}

RenderSystem& EngineRuntime::GetRenderSystem()
{
    return renderSystem_;
}

const SimulationSystem& EngineRuntime::GetSimulationSystem() const
{
    return simulation_;
}

void EngineRuntime::Update(double elapsedSeconds)
{
    if (!started_)
    {
        return;
    }
    const auto previousStep = simulation_.StepCount();
    simulation_.Update(elapsedSeconds);
    if (simulation_.StepCount() != previousStep)
    {
        simulationBinding_.Apply(simulation_);
    }
}

void EngineRuntime::PlaySimulation()
{
    if (started_)
    {
        simulation_.Play();
    }
}
void EngineRuntime::PauseSimulation()
{
    simulation_.Pause();
}
void EngineRuntime::StepSimulation()
{
    if (!started_)
    {
        return;
    }
    simulation_.StepOnce();
    simulationBinding_.Apply(simulation_);
}
void EngineRuntime::ResetSimulation()
{
    if (!started_)
    {
        return;
    }
    simulation_.Reset();
    simulationBinding_.Apply(simulation_);
}
} // namespace buffalo
