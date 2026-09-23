// MIT License - Copyright (c) 2026 AnselZ
#include "core/SceneSimulationBinding.h"
#include "scene/Scene.h"
#include <algorithm>

namespace buffalo
{
void SceneSimulationBinding::Bind(Scene& scene, SimulationSystem& simulation)
{
    Clear();
    auto* models = scene.GetVisibleModels();
    if (!models->empty())
    {
        model_ = models->front();
    }
    if (!model_ || !model_->GetMesh()->IsValid())
    {
        Clear();
        simulation.Initialize({}, {});
        return;
    }
    initialMatrix_ = model_->GetModelMatrix();
    Bounds3 bounds;
    for (const auto& vertex : model_->GetMesh()->vertices)
    {
        bounds.Expand((initialMatrix_ * Vector4::FromPoint(vertex)).XYZ());
    }
    initialCenter_ = bounds.Center();
    const float radius = std::max(0.1f, bounds.Radius());
    RigidBodyDescription body;
    body.initialState = {1, initialCenter_, Vector3(0, 0, 0)};
    body.halfHeight = (bounds.max.y - bounds.min.y) * 0.5f;
    RigidBodySettings settings;
    // Small fall below the original pose keeps the existing camera framing useful.
    groundHeight_ = bounds.min.y - radius * 0.5f;
    settings.groundHeight = groundHeight_;
    simulation.Initialize({body}, settings);
}

void SceneSimulationBinding::Apply(const SimulationSystem& simulation)
{
    if (!model_)
    {
        return;
    }
    simulation.Readback(snapshot_);
    for (const auto& state : snapshot_)
    {
        if (state.id == 1)
        {
            const auto delta = state.position - initialCenter_;
            model_->SetModelMatrix(Matrix4::Translation(delta.x, delta.y, delta.z) * initialMatrix_);
            return;
        }
    }
}

void SceneSimulationBinding::Clear()
{
    model_ = nullptr;
    groundHeight_ = 0;
    snapshot_.clear();
}
} // namespace buffalo
