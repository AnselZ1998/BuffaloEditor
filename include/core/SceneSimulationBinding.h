// MIT License - Copyright (c) 2026 AnselZ
#pragma once

#include "math/Matrix.h"
#include "simulation/SimulationSystem.h"

class Scene;
class Model;

namespace buffalo
{
// Teaching demo adapter: binds the first model; never owned by a solver.
// Runtime must clear this binding before SceneManager destroys the scene.
class SceneSimulationBinding
{
public:
    void Bind(Scene& scene, SimulationSystem& simulation);
    void Apply(const SimulationSystem& simulation);
    void Clear();
    float GroundHeight() const
    {
        return groundHeight_;
    }

private:
    Model* model_ = nullptr;
    Matrix4 initialMatrix_;
    Vector3 initialCenter_{0, 0, 0};
    float groundHeight_ = 0;
    std::vector<RigidBodyState> snapshot_;
};
} // namespace buffalo
