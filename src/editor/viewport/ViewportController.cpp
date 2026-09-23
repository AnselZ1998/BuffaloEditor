// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "editor/viewport/ViewportController.h"
#include "editor/services/EditorLog.h"
#include "core/EngineRuntime.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <iomanip>
#include <sstream>

namespace buffalo::editor
{
ViewportController::ViewportController(EngineRuntime& runtime, EditorLog& log)
    : runtime_(runtime), log_(log)
{
    FrameAll();
}

std::size_t ViewportController::BackendCount() const
{
    return runtime_.GetRenderSystem().BackendCount();
}

const char* ViewportController::BackendName(std::size_t index) const
{
    return runtime_.GetRenderSystem().BackendName(index);
}

std::size_t ViewportController::ActiveBackend() const
{
    return runtime_.GetRenderSystem().ActiveBackend();
}

void ViewportController::SelectBackend(std::size_t index)
{
    if (index < BackendCount() && index != ActiveBackend())
    {
        runtime_.GetRenderSystem().SelectBackend(index);
        log_.Info(std::string("Render backend selected: ") + BackendName(index));
    }
}

ViewportShowFlags ViewportController::GetShowFlags() const
{
    return showFlags_;
}

void ViewportController::SetShowFlags(const ViewportShowFlags& flags)
{
    showFlags_ = flags;
}

void ViewportController::SetSize(int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        width_ = 0;
        height_ = 0;
        return;
    }

    const float scale = std::min({1.0f, 1280.0f / static_cast<float>(width), 720.0f / static_cast<float>(height)});

    width_ = std::max(1, static_cast<int>(static_cast<float>(width) * scale));
    height_ = std::max(1, static_cast<int>(static_cast<float>(height) * scale));
}

void ViewportController::OnInput(const InputEvent& event)
{
    if (!runtime_.IsStarted() || !std::isfinite(event.x) || !std::isfinite(event.y))
    {
        return;
    }

    switch (event.action)
    {
    case InputAction::Orbit:
    {
        yaw_ -= event.x * 4.0f;
        pitch_ = std::clamp(pitch_ + event.y * 4.0f, -1.4f, 1.4f);
        break;
    }

    case InputAction::Pan:
    {
        Vector3 right(std::cos(yaw_), 0.0f, -std::sin(yaw_));
        Vector3 up(-std::sin(yaw_) * std::sin(pitch_), std::cos(pitch_), -std::cos(yaw_) * std::sin(pitch_));

        target_ = target_ - right * (event.x * distance_) + up * (event.y * distance_);
        break;
    }

    case InputAction::Zoom:
    {
        distance_ *= std::exp(-std::clamp(event.x, -20.0f, 20.0f) * 0.12f);
        distance_ = std::clamp(distance_, radius_ * 1.2f, radius_ * 40.0f);
        LogCamera();
        break;
    }

    case InputAction::FrameAll:
    {
        FrameAll();
        LogCamera();
        return;
    }

    case InputAction::NavigationEnd:
    {
        LogCamera();
        break;
    }
    }

    ApplyCameraNavigation();
}

void ViewportController::FrameAll()
{
    if (!runtime_.IsStarted())
    {
        return;
    }

    Scene* scene = runtime_.GetSceneManager().GetCurrentScene();

    if (scene == nullptr)
    {
        return;
    }

    Camera* camera = scene->getCurrentCamera();

    if (camera == nullptr)
    {
        return;
    }

    const Bounds3 bounds = scene->GetWorldBounds();

    if (bounds.valid)
    {
        target_ = bounds.Center();
        radius_ = std::max(0.1f, bounds.Radius());
    }
    else
    {
        target_ = Vector3(0.0f, 0.0f, 0.0f);
        radius_ = 1.0f;
    }

    yaw_ = 0.5f;
    pitch_ = 0.2f;

    const Vector3 backward(
        std::sin(yaw_) * std::cos(pitch_),
        std::sin(pitch_),
        std::cos(yaw_) * std::cos(pitch_));

    const float aspect = height_ > 0
                             ? static_cast<float>(width_) / static_cast<float>(height_)
                             : 1.0f;

    distance_ = camera->FrameSphere(
        target_,
        radius_,
        backward,
        aspect);
}

void ViewportController::ApplyCameraNavigation()
{
    if (!runtime_.IsStarted())
    {
        return;
    }

    Scene* scene = runtime_.GetSceneManager().GetCurrentScene();

    if (scene == nullptr)
    {
        return;
    }

    Camera* camera = scene->getCurrentCamera();

    if (camera == nullptr)
    {
        return;
    }

    Vector3 backward(std::sin(yaw_) * std::cos(pitch_), std::sin(pitch_), std::cos(yaw_) * std::cos(pitch_));

    camera->target = target_;
    camera->position = target_ + backward * distance_;
    camera->up = Vector3(0.0f, 1.0f, 0.0f);

    camera->aspect = height_ > 0
                         ? static_cast<float>(width_) / static_cast<float>(height_)
                         : 1.0f;

    camera->nearZ = std::max(0.01f, radius_ * 0.01f);
    camera->farZ = std::max(1000.0f, radius_ * 100.0f);
    camera->fovYRadians = 1.047197551f;

    camera->UpdateMatrices();
}

ViewportFrame ViewportController::RenderFrame()
{
    if (!runtime_.IsStarted() || BackendCount() == 0 || width_ <= 0 || height_ <= 0)
    {
        return ViewportFrame{};
    }

    ApplyCameraNavigation();

    Scene* scene = runtime_.GetSceneManager().GetCurrentScene();

    if (scene == nullptr)
    {
        return ViewportFrame{};
    }

    try
    {
        return runtime_.GetRenderSystem().Render(*scene, width_, height_);
    }
    catch (const std::exception& error)
    {
        log_.Error(std::string("Render failed: ") + error.what());
        // A failed optional GPU backend must not unwind through the Qt timer.
        // The software backend is registered first by Runtime.
        if (ActiveBackend() != 0)
        {
            SelectBackend(0);
            try
            {
                return runtime_.GetRenderSystem().Render(*scene, width_, height_);
            }
            catch (const std::exception& fallbackError)
            {
                log_.Error(std::string("Software fallback failed: ") + fallbackError.what());
            }
        }
        return ViewportFrame{};
    }
}

void ViewportController::LogCamera()
{
    std::ostringstream message;

    message << std::fixed
            << std::setprecision(2)
            << "Camera | distance " << distance_
            << " | target " << target_.x
            << ", " << target_.y
            << ", " << target_.z;

    log_.Debug(message.str());
}

} // namespace buffalo::editor
