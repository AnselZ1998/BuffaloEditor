// MIT License - Copyright (c) 2026 AnselZ
#include "simulation/SimulationSystem.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace buffalo
{
SimulationSystem::SimulationSystem(std::unique_ptr<IRigidBodySolver> solver, double fixedStep)
    : solver_(std::move(solver)), fixedStep_(fixedStep)
{
    if (!solver_ || !std::isfinite(fixedStep_) || fixedStep_ < 0.000001 || fixedStep_ > 1.0)
    {
        throw std::invalid_argument("Simulation requires a solver and a step between 1us and 1s");
    }
}

void SimulationSystem::Initialize(const std::vector<RigidBodyDescription>& bodies, const RigidBodySettings& settings)
{
    solver_->Initialize(bodies, settings);
    initialBodies_ = bodies;
    settings_ = settings;
    accumulator_ = 0;
    steps_ = 0;
    playing_ = false;
}

void SimulationSystem::Play()
{
    playing_ = HasBodies();
}
void SimulationSystem::Pause()
{
    playing_ = false;
    accumulator_ = 0;
}

void SimulationSystem::Reset()
{
    solver_->Initialize(initialBodies_, settings_);
    Pause();
    steps_ = 0;
}

void SimulationSystem::StepOnce()
{
    Pause();
    if (HasBodies())
    {
        solver_->Step(static_cast<float>(fixedStep_));
        ++steps_;
    }
}

void SimulationSystem::Update(double elapsedSeconds)
{
    if (!playing_ || !std::isfinite(elapsedSeconds) || elapsedSeconds <= 0)
    {
        return;
    }
    accumulator_ += std::min(elapsedSeconds, fixedStep_ * 8.0);
    for (int step = 0; step < 8 && accumulator_ + fixedStep_ * 1e-9 >= fixedStep_; ++step)
    {
        solver_->Step(static_cast<float>(fixedStep_));
        ++steps_;
        accumulator_ = std::max(0.0, accumulator_ - fixedStep_);
    }
}

void SimulationSystem::Readback(std::vector<RigidBodyState>& output) const
{
    solver_->Readback(output);
}
} // namespace buffalo
