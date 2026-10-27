// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include "render/IRenderBackend.h"
#include "core/ILogger.h"
#include <memory>
#include <vector>

namespace buffalo
{
// Owns backend instances. No Qt, editor services or window presentation here.
class RenderSystem
{
public:
    void SetLogger(ILogger* logger)
    {
        logger_ = logger;
    }
    void AddBackend(std::unique_ptr<IRenderBackend> backend);
    std::size_t BackendCount() const;
    const char* BackendName(std::size_t index) const;
    std::size_t ActiveBackend() const;
    void SelectBackend(std::size_t index);
    RenderFrame Render(Scene& scene, int width, int height);
    void ShutDown();

private:
    std::vector<std::unique_ptr<IRenderBackend>> backends_;
    std::size_t activeBackend_ = 0;
    ILogger* logger_ = nullptr;
    int lastWidth_ = 0;
    int lastHeight_ = 0;
};
} // namespace buffalo
