// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include "render/IRenderBackend.h"
#include "render/software/SoftwareRenderer.h"

namespace buffalo
{
// Adapts the existing software renderer without changing its implementation.
class SoftwareRenderBackend final : public IRenderBackend
{
public:
    SoftwareRenderBackend() = default;
    ~SoftwareRenderBackend() override;
    SoftwareRenderBackend(const SoftwareRenderBackend&) = delete;
    SoftwareRenderBackend& operator=(const SoftwareRenderBackend&) = delete;
    const char* Name() const override;
    RenderFrame Render(Scene& scene, int width, int height) override;

private:
    SoftwareRenderer renderer_;
    bool started_ = false;
};
} // namespace buffalo
