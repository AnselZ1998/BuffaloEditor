// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "render/software/SoftwareRenderBackend.h"
#include "scene/Scene.h"

namespace buffalo
{
SoftwareRenderBackend::~SoftwareRenderBackend()
{
    renderer_.ShutDown();
}

const char* SoftwareRenderBackend::Name() const
{
    return "Software";
}

RenderFrame SoftwareRenderBackend::Render(Scene& scene, int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        return {};
    }

    if (!started_)
    {
        started_ = renderer_.StartUp(width, height);
        if (!started_)
        {
            return {};
        }
    }
    else
    {
        renderer_.Resize(width, height);
    }

    renderer_.SetCameraToRender(scene.getCurrentCamera());
    renderer_.BeginFrame(0xFF25282Du);
    auto* models = scene.GetVisibleModels();
    const auto lighting = scene.Lighting();
    while (!models->empty())
    {
        models->front()->GetMaterial()->lights = lighting;
        renderer_.drawTriangularMesh(models->front());
        models->pop();
    }

    const auto* buffer = renderer_.GetColorBuffer();
    if (!buffer || !buffer->IsValid())
    {
        return {};
    }
    return RenderFrame{
        buffer->Data(), 
        buffer->GetWidth(), 
        buffer->GetHeight(), 
        buffer->GetPitchBytes(), 
        PixelFormat::BGRA8
    };
}
} // namespace buffalo
