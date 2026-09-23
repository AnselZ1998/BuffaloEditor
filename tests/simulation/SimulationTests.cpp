// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "simulation/SimulationSystem.h"
#include "simulation/rigidbody/cpu/CpuRigidBodySolver.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace buffalo;

namespace
{
void Require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void Near(double actual, double expected, double tolerance, const char* message)
{
    Require(std::isfinite(actual) && std::abs(actual - expected) <= tolerance, message);
}

template <typename Function>
void Rejects(Function action, const char* message)
{
    try
    {
        action();
    }
    catch (const std::invalid_argument&)
    {
        return;
    }
    throw std::runtime_error(message);
}

RigidBodyDescription Body(RigidBodyId id, Vector3 position, Vector3 velocity = {})
{
    RigidBodyDescription body;
    body.initialState = {id, position, velocity};
    return body;
}

std::vector<RigidBodyState> Read(const IRigidBodySolver& solver)
{
    std::vector<RigidBodyState> states;
    solver.Readback(states);
    return states;
}

std::vector<RigidBodyState> Read(const SimulationSystem& simulation)
{
    std::vector<RigidBodyState> states;
    simulation.Readback(states);
    return states;
}

void TestFreeFallAndOwnership()
{
    CpuRigidBodySolver solver;
    RigidBodySettings settings;
    settings.groundEnabled = false;
    auto dynamic = Body(17, {1, 20, 3}, {2, 1, -1});
    auto massive = dynamic;
    massive.initialState.id = 18;
    massive.inverseMass = 0.01f;
    auto fixed = Body(99, {-2, 4, 8}, {3, -9, 2});
    fixed.inverseMass = 0;
    solver.Initialize({dynamic, massive, fixed}, settings);

    constexpr int steps = 200;
    constexpr float dt = 0.01f;
    for (int step = 0; step < steps; ++step)
    {
        solver.Step(dt);
    }
    const auto states = Read(solver);
    Require(states.size() == 3 && states[0].id == 17 && states[2].id == 99,
        "Readback preserves body identities");
    const double time = steps * static_cast<double>(dt);
    // Closed-form result of constant acceleration with semi-implicit Euler.
    const double expectedY = 20 + time + settings.gravity.y * dt * dt * steps * (steps + 1) / 2;
    Near(states[0].position.y, expectedY, 0.0005, "Free fall follows the discrete integration solution");
    Near(states[0].velocity.y, 1 + settings.gravity.y * time, 0.0002, "Gravity accumulates velocity correctly");
    Near(states[0].position.x, 1 + 2 * time, 0.0001, "Horizontal motion preserves velocity");
    Near(states[0].position.z, 3 - time, 0.0001, "All translation components advance");
    Near((states[0].position - states[1].position).Length(), 0, 0.00001,
        "Free-fall acceleration is independent of positive mass");
    Near((states[2].position - fixed.initialState.position).Length(), 0, 0,
        "Static bodies do not move even with nonzero stored velocity");

    auto snapshot = states;
    snapshot[0].position.y = -1000;
    solver.Readback(snapshot);
    Near(snapshot[0].position.y, states[0].position.y, 0, "Readback copies solver state into caller-owned storage");
    solver.Step(dt);
    Near(snapshot[0].position.y, states[0].position.y, 0, "Later steps do not mutate existing host snapshots");
}

void TestGroundContact()
{
    CpuRigidBodySolver solver;
    RigidBodySettings settings;
    settings.gravity = {0, 0, 0};
    settings.groundHeight = 2;
    auto body = Body(1, {0, 2.75f, 0}, {3, -4, 0});
    body.restitution = 0.5f;
    solver.Initialize({body}, settings);
    solver.Step(0.125f);
    auto states = Read(solver);
    Near(states[0].position.y, 2.5, 0.000001, "Ground contact uses both plane height and body extent");
    Near(states[0].velocity.y, 2, 0.000001, "Restitution reflects inward normal velocity");
    Near(states[0].velocity.x, 3, 0, "Frictionless contact preserves tangential velocity");

    body.initialState.position.y = 2.25f;
    body.initialState.velocity = {0, 1, 0};
    solver.Initialize({body}, settings);
    solver.Step(0.01f);
    Near(Read(solver)[0].velocity.y, 1, 0, "Penetration correction does not reverse separating velocity");

    settings.gravity = {0, -9.81f, 0};
    body.initialState.position = {0, 5, 0};
    body.initialState.velocity = {};
    solver.Initialize({body}, settings);
    for (int step = 0; step < 1200; ++step)
    {
        solver.Step(1.0f / 120.0f);
        states = Read(solver);
        Require(states[0].position.y >= 2.5f, "Falling bodies never penetrate the ground");
    }
    Near(states[0].position.y, 2.5, 0.000001, "Repeated dissipative contacts settle on the plane");
    Near(states[0].velocity.y, 0, 0, "Resting contact does not bounce from gravity each step");
}

void TestPlaybackAndScheduling()
{
    constexpr double dt = 0.01;
    SimulationSystem simulation(std::make_unique<CpuRigidBodySolver>(), dt);
    RigidBodySettings settings;
    settings.groundEnabled = false;
    const auto body = Body(7, {0, 10, 0});
    simulation.Initialize({body}, settings);
    Require(simulation.HasBodies() && !simulation.IsPlaying(), "New simulations start paused");
    simulation.Update(1);
    Require(simulation.StepCount() == 0, "Paused updates cannot advance simulation");
    simulation.Play();
    simulation.Update(dt * 0.75);
    Require(simulation.StepCount() == 0, "Substep elapsed time waits for a full fixed step");
    simulation.Pause();
    simulation.Play();
    simulation.Update(dt * 0.5);
    Require(simulation.StepCount() == 0, "Pause discards the previous wall-time remainder");
    simulation.Update(dt * 0.5);
    Require(simulation.StepCount() == 1, "Resumed simulation accumulates fresh elapsed time");
    simulation.StepOnce();
    Require(simulation.StepCount() == 2 && !simulation.IsPlaying(), "Single step advances once and pauses playback");
    Near(simulation.Time(), 2 * dt, 1e-12, "Simulation time counts completed fixed steps");
    simulation.Reset();
    Require(!simulation.IsPlaying() && simulation.StepCount() == 0 && simulation.Time() == 0,
        "Reset clears playback and time");
    Near(Read(simulation)[0].position.y, 10, 0, "Reset restores the initial state");

    simulation.Play();
    simulation.Update(100);
    Require(simulation.StepCount() == 8, "A stalled UI update is limited to eight simulation steps");
    simulation.Update(dt * 0.25);
    Require(simulation.StepCount() == 8, "Excess wall time is dropped instead of accumulating a backlog");
    simulation.Update(0);
    simulation.Update(-1);
    simulation.Update(std::numeric_limits<double>::infinity());
    simulation.Update(std::numeric_limits<double>::quiet_NaN());
    Require(simulation.StepCount() == 8, "Invalid elapsed time cannot corrupt playback");

    simulation.Initialize({}, settings);
    simulation.Play();
    simulation.StepOnce();
    Require(!simulation.HasBodies() && !simulation.IsPlaying() && simulation.StepCount() == 0,
        "An empty world does not play or single-step");
    Require(Read(simulation).empty(), "Reinitialization replaces previous bodies");
}

void TestFrameRateIndependence()
{
    SimulationSystem fine(std::make_unique<CpuRigidBodySolver>(), 0.01);
    SimulationSystem coarse(std::make_unique<CpuRigidBodySolver>(), 0.01);
    RigidBodySettings settings;
    settings.groundEnabled = false;
    const auto body = Body(12, {2, 10, -3}, {1, 0, 2});
    fine.Initialize({body}, settings);
    coarse.Initialize({body}, settings);
    fine.Play();
    coarse.Play();
    for (int frame = 0; frame < 160; ++frame)
    {
        fine.Update(0.0025);
    }
    for (int frame = 0; frame < 10; ++frame)
    {
        coarse.Update(0.04);
    }
    Require(fine.StepCount() == 40 && coarse.StepCount() == 40,
        "Different display frame rates produce equal simulation step counts");
    Near((Read(fine)[0].position - Read(coarse)[0].position).Length(), 0, 0.000001,
        "Equal elapsed time produces equal state without frame-rate-dependent integration");
}

void TestInvalidInputs()
{
    const float nan = std::numeric_limits<float>::quiet_NaN();
    Rejects([]
    {
        SimulationSystem invalid(nullptr);
    }, "A solver is required");
    Rejects([]
    {
        SimulationSystem invalid(std::make_unique<CpuRigidBodySolver>(), 0);
    }, "A fixed step must be positive");
    Rejects([nan]
    {
        SimulationSystem invalid(std::make_unique<CpuRigidBodySolver>(), nan);
    }, "A fixed step must be finite");
    CpuRigidBodySolver solver;
    const auto body = Body(3, {0, 3, 0});
    RigidBodySettings settings;
    solver.Initialize({body}, settings);
    Rejects([&]
    {
        solver.Initialize({body, body}, settings);
    }, "Duplicate IDs must be rejected");
    auto invalidBody = body;
    invalidBody.inverseMass = -1;
    Rejects([&]
    {
        solver.Initialize({invalidBody}, settings);
    }, "Negative inverse mass must be rejected");
    invalidBody = body;
    invalidBody.initialState.velocity.x = nan;
    Rejects([&]
    {
        solver.Initialize({invalidBody}, settings);
    }, "Non-finite body state must be rejected");
    invalidBody = body;
    invalidBody.restitution = 1.1f;
    Rejects([&]
    {
        solver.Initialize({invalidBody}, settings);
    }, "Restitution cannot create energy");
    auto invalidSettings = settings;
    invalidSettings.gravity.y = nan;
    Rejects([&]
    {
        solver.Initialize({body}, invalidSettings);
    }, "Non-finite gravity must be rejected");
    invalidSettings = settings;
    invalidSettings.bounceThreshold = -1;
    Rejects([&]
    {
        solver.Initialize({body}, invalidSettings);
    }, "Negative bounce threshold must be rejected");
    Rejects([&]
    {
        solver.Step(0);
    }, "Zero solver step must be rejected");
    Rejects([&]
    {
        solver.Step(nan);
    }, "Non-finite solver step must be rejected");
    const auto states = Read(solver);
    Require(states.size() == 1 && states[0].id == 3, "Rejected inputs preserve the previous valid world");
    Near(states[0].position.y, 3, 0, "Rejected inputs cannot mutate valid state");
}
} // namespace

int main()
{
    try
    {
        TestFreeFallAndOwnership();
        TestGroundContact();
        TestPlaybackAndScheduling();
        TestFrameRateIndependence();
        TestInvalidInputs();
        std::cout << "Passed: fixed-step CPU free fall, ground contacts, state ownership, playback and validation.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
