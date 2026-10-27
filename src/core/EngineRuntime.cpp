// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "core/EngineRuntime.h"
#include "render/software/SoftwareRenderBackend.h"
#include "simulation/rigidbody/cpu/CpuRigidBodySolver.h"
#include <memory>

namespace buffalo
{
EngineRuntime::EngineRuntime(): simulation_(std::make_unique<CpuRigidBodySolver>())
{
}

EngineRuntime::~EngineRuntime()
{
    ShutDown();
}

void EngineRuntime::SetLogger(ILogger* logger)
{
    logger_ = logger;
    renderSystem_.SetLogger(logger);
}

void EngineRuntime::RebindSimulation()
{
    if (logger_)
    {
        logger_->Debug("Runtime: rebinding simulation to current Scene.");
    }
    simulation_.Pause();
    simulationBinding_.Clear();
    Scene* scene = sceneManager_.GetCurrentScene();
    simulationScene_ = scene ? scene->Id() : 0;
    if (scene)
    {
        simulationBinding_.Bind(*scene, simulation_);
    }
    else
    {
        simulation_.Initialize({}, {});
    }
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
        RebindSimulation();
        if (logger_)
        {
            logger_->Info("Runtime started.");
            if (!sceneManager_.GetCurrentScene()->CheckIfEmpty())
            {
                logger_->Warning("Default Scene contains no renderable model.");
            }
        }

        // TODO: add other render here
        // When we Create QOpenGLContext, use AddBackend() register OpenGLRenderBackend

        started_ = true;
        return true;
    }
    catch (...)
    {
        if (logger_)
        {
            logger_->Error("Runtime startup failed.");
        }
        ShutDown();
        throw;
    }
}

void EngineRuntime::ShutDown()
{
    if (started_ && logger_)
    {
        logger_->Info("Runtime shutdown.");
    }
    simulation_.Pause();
    simulationBinding_.Clear();
    simulation_.Initialize({}, {});
    // Backends may borrow the scene camera; release them before the scene.
    renderSystem_.ShutDown();
    // SceneManager makes repeated shutdown safe and owns Scene cleanup.
    sceneManager_.ShutDown();
    started_ = false;
    simulationScene_ = 0;
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
    Scene* scene = sceneManager_.GetCurrentScene();
    if ((scene ? scene->Id() : 0) != simulationScene_)
    {
        RebindSimulation();
    }
    if (scene)
    {
        scene->UpdateLights(elapsedSeconds);
    }
    simulation_.Update(elapsedSeconds);
    if (simulation_.StepCount() != previousStep)
    {
        simulationBinding_.Apply(simulation_);
    }
}

void EngineRuntime::PlaySimulation()
{
    Scene* scene = sceneManager_.GetCurrentScene();
    if ((scene ? scene->Id() : 0) != simulationScene_)
    {
        RebindSimulation();
    }
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
    Scene* scene = sceneManager_.GetCurrentScene();
    if ((scene ? scene->Id() : 0) != simulationScene_)
    {
        RebindSimulation();
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
    Scene* scene = sceneManager_.GetCurrentScene();
    if ((scene ? scene->Id() : 0) != simulationScene_)
    {
        RebindSimulation();
    }
    simulation_.Reset();
    simulationBinding_.Apply(simulation_);
}
} // namespace buffalo
