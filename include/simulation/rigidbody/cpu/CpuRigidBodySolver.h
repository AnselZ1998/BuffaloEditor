// MIT License - Copyright (c) 2026 AnselZ
#pragma once

#include "simulation/rigidbody/IRigidBodySolver.h"

namespace buffalo
{
class CpuRigidBodySolver final : public IRigidBodySolver
{
public:
    const char* Name() const override
    {
        return "CPU rigid body";
    }
    void Initialize(const std::vector<RigidBodyDescription>& bodies,
        const RigidBodySettings& settings) override;
    void Step(float seconds) override;
    void Readback(std::vector<RigidBodyState>& output) const override;

private:
    std::vector<RigidBodyDescription> bodies_;
    std::vector<RigidBodyState> states_;
    RigidBodySettings settings_;
};
} // namespace buffalo
