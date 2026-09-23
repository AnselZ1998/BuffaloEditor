// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include "editor/viewport/IViewport.h"
#include "math/Vector.h"

namespace buffalo
{
class EngineRuntime;
}

namespace buffalo::editor
{
class EditorLog;

// Translates editor intentions to Runtime calls. Owns only navigation/show state.
class ViewportController final : public IViewport
{
public:
    ViewportController(EngineRuntime& runtime, EditorLog& log);
    void OnInput(const InputEvent& event) override;
    void SetSize(int width, int height) override;
    ViewportFrame RenderFrame() override;
    std::size_t BackendCount() const override;
    const char* BackendName(std::size_t index) const override;
    std::size_t ActiveBackend() const override;
    void SelectBackend(std::size_t index) override;
    ViewportShowFlags GetShowFlags() const override;
    void SetShowFlags(const ViewportShowFlags& flags) override;

private:
    void FrameAll();
    void ApplyCameraNavigation();
    void LogCamera();

    EngineRuntime& runtime_;
    EditorLog& log_;
    ViewportShowFlags showFlags_;
    int width_ = 0;
    int height_ = 0;
    float yaw_ = 0.5f;
    float pitch_ = 0.2f;
    float distance_ = 6.0f;
    float radius_ = 1.0f;
    Vector3 target_{0.0f, 0.0f, 0.0f};
};
} // namespace buffalo::editor
