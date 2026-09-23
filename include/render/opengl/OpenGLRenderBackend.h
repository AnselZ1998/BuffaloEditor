// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include "render/IRenderBackend.h"

#include <memory>

class QOpenGLContext;

namespace buffalo
{
// Optional Qt OpenGL integration, linked separately from BuffaloRuntime.
// The backend owns an offscreen context sharing textures with shareContext.
// Construct/render/destroy on the GUI thread while the application is alive.
// Initialization and rendering errors throw std::runtime_error for the host
// to report; an unavailable device does not invalidate the software backend.
class OpenGLRenderBackend final : public IRenderBackend
{
public:
    explicit OpenGLRenderBackend(QOpenGLContext* shareContext);
    ~OpenGLRenderBackend() override;
    OpenGLRenderBackend(const OpenGLRenderBackend&) = delete;
    OpenGLRenderBackend& operator=(const OpenGLRenderBackend&) = delete;

    const char* Name() const override;
    RenderFrame Render(Scene& scene, int width, int height) override;

    // Mesh contents are immutable between invalidations. Transforms may change
    // every frame without uploads. Call before replacing/deforming mesh data.
    void InvalidateMeshCache();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace buffalo
