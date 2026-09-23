// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "render/RenderSystem.h"
#include <stdexcept>
#include <utility>

namespace buffalo
{
void RenderSystem::AddBackend(std::unique_ptr<IRenderBackend> backend)
{
    if (!backend)
    {
        throw std::invalid_argument("Render backend cannot be null");
    }
    backends_.push_back(std::move(backend));
}

std::size_t RenderSystem::BackendCount() const
{
    return backends_.size();
}

const char* RenderSystem::BackendName(std::size_t index) const
{
    return index < backends_.size() ? backends_[index]->Name() : "Unavailable";
}

std::size_t RenderSystem::ActiveBackend() const
{
    return activeBackend_;
}

void RenderSystem::SelectBackend(std::size_t index)
{
    if (index < backends_.size())
    {
        activeBackend_ = index;
    }
}

RenderFrame RenderSystem::Render(Scene& scene, int width, int height)
{
    if (backends_.empty() || width <= 0 || height <= 0)
    {
        return {};
    }
    return backends_[activeBackend_]->Render(scene, width, height);
}

void RenderSystem::ShutDown()
{
    backends_.clear();
    activeBackend_ = 0;
}
} // namespace buffalo
