// MIT License - Copyright (c) 2026 AnselZ
#pragma once

#include "simulation/rigidbody/IRigidBodySolver.h"
#include <cstdint>
#include <memory>

namespace buffalo
{
// Scheduling and playback only. Scene synchronization lives outside simulation/.
class SimulationSystem
{
public:
    explicit SimulationSystem(std::unique_ptr<IRigidBodySolver> solver,
        double fixedStep = 1.0 / 120.0);
    void Initialize(const std::vector<RigidBodyDescription>& bodies, const RigidBodySettings& settings);
    void Play();
    void Pause();
    void Reset();
    void StepOnce();
    // At most eight steps per update; drops excess wall time to keep the UI responsive.
    void Update(double elapsedSeconds);
    void Readback(std::vector<RigidBodyState>& output) const;

    bool IsPlaying() const
    {
        return playing_;
    }
    bool HasBodies() const
    {
        return !initialBodies_.empty();
    }
    double Time() const
    {
        return static_cast<double>(steps_) * fixedStep_;
    }
    std::uint64_t StepCount() const
    {
        return steps_;
    }
    double FixedStep() const
    {
        return fixedStep_;
    }
    const char* SolverName() const
    {
        return solver_->Name();
    }

private:
    std::unique_ptr<IRigidBodySolver> solver_;
    std::vector<RigidBodyDescription> initialBodies_;
    RigidBodySettings settings_;
    double fixedStep_;
    double accumulator_ = 0;
    std::uint64_t steps_ = 0;
    bool playing_ = false;
};
} // namespace buffalo
