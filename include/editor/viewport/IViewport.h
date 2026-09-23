// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include "editor/viewport/InputEvent.h"
#include "editor/viewport/ViewportFrame.h"
#include "editor/viewport/ViewportShowFlags.h"
#include <cstddef>

namespace buffalo::editor
{
// The preview panel holds only this interface to the engine.
// Device input, scene ownership and renderer implementations stay outside UI.
class IViewport
{
public:
    virtual ~IViewport() = default;
    virtual void OnInput(const InputEvent& event) = 0;
    virtual void SetSize(int width, int height) = 0;
    virtual ViewportFrame RenderFrame() = 0;
    virtual std::size_t BackendCount() const = 0;
    virtual const char* BackendName(std::size_t index) const = 0;
    virtual std::size_t ActiveBackend() const = 0;
    virtual void SelectBackend(std::size_t index) = 0;
    virtual ViewportShowFlags GetShowFlags() const = 0;
    virtual void SetShowFlags(const ViewportShowFlags& flags) = 0;
};
} // namespace buffalo::editor
