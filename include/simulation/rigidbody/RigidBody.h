// MIT License - Copyright (c) 2026 AnselZ
#pragma once

#include "math/Vector.h"
#include <cstdint>

namespace buffalo
{
using RigidBodyId = std::uint32_t;

// v1 translation-only body against a horizontal plane, not a full 6-DOF body.
struct RigidBodyState
{
    RigidBodyId id = 0;
    Vector3 position{0, 0, 0};
    Vector3 velocity{0, 0, 0};
};

struct RigidBodyDescription
{
    RigidBodyState initialState;
    float inverseMass = 1.0f; // Zero means static.
    float halfHeight = 0.5f;
    float restitution = 0.4f;
};

struct RigidBodySettings
{
    Vector3 gravity{0, -9.81f, 0};
    bool groundEnabled = true;
    float groundHeight = 0.0f;
    float bounceThreshold = 0.2f; // Slow contacts stop instead of bouncing forever.
};
} // namespace buffalo
