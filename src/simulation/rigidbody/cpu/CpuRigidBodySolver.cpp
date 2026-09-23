// MIT License - Copyright (c) 2026 AnselZ
#include "simulation/rigidbody/cpu/CpuRigidBodySolver.h"

#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace buffalo
{
namespace
{
bool Finite(const Vector3& v)
{
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
} // namespace

void CpuRigidBodySolver::Initialize(const std::vector<RigidBodyDescription>& bodies,
    const RigidBodySettings& settings)
{
    if (!Finite(settings.gravity) || !std::isfinite(settings.groundHeight) ||
        !std::isfinite(settings.bounceThreshold) || settings.bounceThreshold < 0)
    {
        throw std::invalid_argument("Invalid rigid body world settings");
    }
    std::unordered_set<RigidBodyId> ids;
    std::vector<RigidBodyState> states;
    states.reserve(bodies.size());
    for (const auto& body : bodies)
    {
        if (!ids.insert(body.initialState.id).second || !Finite(body.initialState.position) ||
            !Finite(body.initialState.velocity) || !std::isfinite(body.inverseMass) || body.inverseMass < 0 ||
            !std::isfinite(body.halfHeight) || body.halfHeight < 0 ||
            !std::isfinite(body.restitution) || body.restitution < 0 || body.restitution > 1)
        {
            throw std::invalid_argument("Invalid rigid body description or duplicate ID");
        }
        states.push_back(body.initialState);
    }
    bodies_ = bodies;
    states_ = std::move(states);
    settings_ = settings;
}

void CpuRigidBodySolver::Step(float seconds)
{
    if (!std::isfinite(seconds) || seconds <= 0)
    {
        throw std::invalid_argument("Simulation step must be finite and positive");
    }
    for (std::size_t i = 0; i < states_.size(); ++i)
    {
        const auto& body = bodies_[i];
        if (body.inverseMass == 0)
        {
            continue;
        }
        auto& state = states_[i];
        // Semi-implicit Euler: update velocity before position.
        state.velocity += settings_.gravity * seconds;
        state.position += state.velocity * seconds;
        const float contactHeight = settings_.groundHeight + body.halfHeight;
        if (settings_.groundEnabled && state.position.y <= contactHeight)
        {
            state.position.y = contactHeight;
            if (state.velocity.y < 0)
            {
                // Include gravity's one-step contribution so resting contacts stay at rest.
                const float threshold = settings_.bounceThreshold + std::abs(settings_.gravity.y * seconds);
                state.velocity.y = -state.velocity.y > threshold ? -state.velocity.y * body.restitution : 0.0f;
            }
        }
    }
}

void CpuRigidBodySolver::Readback(std::vector<RigidBodyState>& output) const
{
    output = states_;
}
} // namespace buffalo
