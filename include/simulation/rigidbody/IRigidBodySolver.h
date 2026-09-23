// MIT License - Copyright (c) 2026 AnselZ
#pragma once

#include "simulation/rigidbody/RigidBody.h"
#include <vector>

namespace buffalo
{
// No Scene, renderer, Qt or device types. The solver owns its working state.
class IRigidBodySolver
{
public:
    virtual ~IRigidBodySolver() = default;
    virtual const char* Name() const = 0;
    // Replaces the world and restores initial states. IDs must be unique.
    virtual void Initialize(const std::vector<RigidBodyDescription>& bodies,
        const RigidBodySettings& settings) = 0;
    // Ordered synchronous submission in v1; no host snapshot required here.
    virtual void Step(float seconds) = 0;
    // Explicit host readback boundary. Future GPU implementations may block here.
    // Caller owns the result; no borrowed device pointers or hidden Scene writes.
    virtual void Readback(std::vector<RigidBodyState>& output) const = 0;
};
} // namespace buffalo
