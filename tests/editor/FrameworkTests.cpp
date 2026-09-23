// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/viewport/ViewportController.h"
#include "render/software/SoftwareRenderBackend.h"
#include "core/EngineRuntime.h"
#include "editor/services/EditorLog.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace buffalo;
using namespace buffalo::editor;

void Require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

class ProbeBackend final : public IRenderBackend
{
public:
    const char* Name() const override
    {
        return "Probe";
    }
    ViewportFrame Render(Scene& scene, int width, int height) override
    {
        ++calls;
        camera = scene.getCurrentCamera();
        position = camera->position;
        aspect = camera->aspect;
        lastWidth = width;
        lastHeight = height;
        return {};
    }
    Camera* camera = nullptr;
    Vector3 position{0, 0, 0};
    float aspect = 0;
    int calls = 0, lastWidth = 0, lastHeight = 0;
};

std::uint64_t Hash(const ViewportFrame& frame)
{
    std::uint64_t hash = 1469598103934665603ull;
    for (int y = 0; y < frame.height; ++y)
    {
        auto* row = reinterpret_cast<const std::uint32_t*>(
            reinterpret_cast<const unsigned char*>(frame.pixels) + y * frame.rowPitchBytes);
        for (int x = 0; x < frame.width; ++x)
        {
            hash ^= row[x];
            hash *= 1099511628211ull;
        }
    }
    return hash;
}

int main()
{
    try
    {
        std::uint32_t pixel = 0;
        Require(!ViewportFrame{&pixel, 3, 1, 8}.IsValid(), "Reject undersized row stride");
        Require(ViewportFrame{&pixel, 1, 1, 8}.IsValid(), "Accept padded row stride");
        Require(!ViewportFrame{}.IsValid(), "Reject empty frame");

        EditorLog log;
        EngineRuntime runtime;
        Require(runtime.StartUp(), "Runtime starts the existing scene");
        Require(runtime.StartUp() && runtime.GetRenderSystem().BackendCount() == 1, "Repeated startup does not duplicate scenes or backends");
        Scene* scene = runtime.GetSceneManager().GetCurrentScene();
        Require(runtime.GetSimulationSystem().HasBodies() && !runtime.GetSimulationSystem().IsPlaying(),
            "Simulation binds an existing model and starts paused");
        const auto initialCenter = scene->GetWorldBounds().Center();
        runtime.Update(1.0);
        Require(runtime.GetSimulationSystem().StepCount() == 0, "Paused runtime never advances physics");
        runtime.StepSimulation();
        Require(runtime.GetSimulationSystem().StepCount() == 1 && scene->GetWorldBounds().Center().y < initialCenter.y,
            "Single step synchronizes simulated position into the existing Scene model");
        runtime.ResetSimulation();
        Require(runtime.GetSimulationSystem().StepCount() == 0 &&
                    (scene->GetWorldBounds().Center() - initialCenter).Length() < 0.00001f,
            "Reset restores scene pose as well as simulation state");
        {
            ViewportController temporary(runtime, log);
        }
        Require(runtime.GetSceneManager().GetCurrentScene() == scene && runtime.IsStarted(), "Destroying a controller preserves Runtime scene ownership");
        runtime.GetRenderSystem().ShutDown();
        ViewportController controller(runtime, log);
        auto probe = std::make_unique<ProbeBackend>();
        auto* observed = probe.get();
        runtime.GetRenderSystem().AddBackend(std::move(probe));
        runtime.GetRenderSystem().AddBackend(std::make_unique<SoftwareRenderBackend>());
        Require(controller.BackendCount() == 2, "Backend registration");
        controller.SetSize(0, 100);
        controller.RenderFrame();
        Require(observed->calls == 0, "Collapsed viewport must not render");

        controller.SetSize(320, 240);
        controller.RenderFrame();
        Require(observed->calls == 1 && observed->camera, "Existing scene must reach backend");
        Camera* sceneCamera = observed->camera;
        Vector3 before = observed->position;
        controller.OnInput({InputAction::Orbit, .12f, -.07f});
        controller.RenderFrame();
        Require(observed->camera == sceneCamera, "Input must update the SAME scene camera");
        Require((observed->position - before).Length() > .01f, "Orbit changes camera position");
        before = observed->camera->target;
        controller.OnInput({InputAction::Pan, .1f, .1f});
        controller.RenderFrame();
        Require((observed->camera->target - before).Length() > .01f, "Pan changes target");
        controller.OnInput({InputAction::Zoom, 100000.f});
        controller.OnInput({InputAction::Orbit, 0, 10000.f});
        controller.RenderFrame();
        Require(std::isfinite(observed->position.x) && std::isfinite(observed->position.y),
            "Extreme navigation remains finite");
        controller.SetSize(2560, 1440);
        controller.RenderFrame();
        Require(observed->lastWidth == 1280 && observed->lastHeight == 720, "CPU resolution cap");
        Require(std::abs(observed->aspect - 16.f / 9) < .001f, "Resize updates projection aspect");

        controller.SelectBackend(999);
        Require(controller.ActiveBackend() == 0, "Invalid backend selection preserves current backend");
        controller.SelectBackend(1);
        controller.SetSize(320, 240);
        controller.OnInput({InputAction::FrameAll});
        ViewportFrame first = controller.RenderFrame();
        Require(first.IsValid() && first.width == 320 && first.height == 240, "Real software frame");
        int shaded = 0;
        for (int i = 0; i < first.width * first.height; ++i)
        {
            if (first.pixels[i] != 0xFF25282Du)
            {
                ++shaded;
            }
        }
        Require(shaded > 100, "The original renderer must draw the actual model, not only background");
        const Bounds3 originalBounds = scene->GetWorldBounds();
        const auto gridHash = Hash(first);
        Require(!controller.GetShowFlags().grid, "Grid follows the current disabled default");
        controller.SetShowFlags({true});
        Require(Hash(controller.RenderFrame()) == gridHash, "Editor grid cannot change Runtime pixels");
        const Bounds3 hiddenGridBounds = scene->GetWorldBounds();
        Require((originalBounds.Center() - hiddenGridBounds.Center()).Length() < 0.0001f && std::abs(originalBounds.Radius() - hiddenGridBounds.Radius()) < 0.0001f, "Grid does not affect Scene bounds");
        controller.SetShowFlags({false});
        auto originalHash = gridHash; // Consume borrowed memory before the next render.
        controller.OnInput({InputAction::Orbit, .15f, 0});
        auto second = controller.RenderFrame();
        Require(Hash(second) != originalHash, "Camera input must change real rendered pixels");
        controller.SetSize(400, 180);
        Require(controller.RenderFrame().width == 400, "Software renderer resizes");
        controller.SetSize(0, 0);
        Require(!controller.RenderFrame().IsValid(), "No stale frame on zero-size viewport");

        for (int i = 0; i < 250; ++i)
        {
            log.Write("test");
        }
        Require(log.Lines().size() == 200, "Log storage is bounded");
        runtime.ShutDown();
        runtime.ShutDown();
        Require(!runtime.IsStarted() && runtime.GetRenderSystem().BackendCount() == 0, "Runtime shutdown is idempotent and clears owned backends");
        Require(!runtime.GetSimulationSystem().HasBodies() && !runtime.GetSimulationSystem().IsPlaying(),
            "Shutdown clears simulated state and scene bindings");
        Require(!controller.RenderFrame().IsValid(), "A controller cannot render a stopped Runtime");
        Require(runtime.StartUp(), "Runtime can restart without changing SceneManager");
        std::cout << "Passed: renderer selection, scene camera input, resize, actual software pixels and frame contract.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
