// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "render/RenderSystem.h"
#include <stdexcept>
#include <utility>
#include "scene/Scene.h"

namespace buffalo
{
void RenderSystem::AddBackend(std::unique_ptr<IRenderBackend> backend)
{
    if (!backend)
    {
        if (logger_)
        {
            logger_->Error("Render: null backend rejected.");
        }
        throw std::invalid_argument("Render backend cannot be null");
    }
    backends_.push_back(std::move(backend));
    if (logger_)
    {
        logger_->Info(std::string("Render backend registered: ") + backends_.back()->Name());
    }
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
        if (logger_)
        {
            logger_->Info(std::string("Render backend selected: ") + BackendName(index));
        }
    }
    else if (logger_)
    {
        logger_->Warning("Render: invalid backend index.");
    }
}

RenderFrame RenderSystem::Render(Scene& scene, int width, int height)
{
    if (backends_.empty() || width <= 0 || height <= 0)
    {
        return {};
    }
    if (logger_ && (width != lastWidth_ || height != lastHeight_))
    {
        logger_->Debug("Render target: " + std::to_string(width) + " x " + std::to_string(height));
    }
    lastWidth_ = width;
    lastHeight_ = height;
    try
    {
        return backends_[activeBackend_]->Render(scene, width, height);
    }
    catch (const std::exception& error)
    {
        if (logger_)
        {
            logger_->Error(std::string("Render failed: ") + error.what());
        }
        throw;
    }
}

void RenderSystem::ShutDown()
{
    backends_.clear();
    activeBackend_ = 0;
}
} // namespace buffalo
