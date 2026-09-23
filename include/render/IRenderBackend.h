// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include "render/RenderFrame.h"

class Scene;

namespace buffalo
{
class IRenderBackend
{
public:
    virtual ~IRenderBackend() = default;
    virtual const char* Name() const = 0;
    virtual RenderFrame Render(Scene& scene, int width, int height) = 0;
};
} // namespace buffalo
