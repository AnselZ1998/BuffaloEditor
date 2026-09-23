// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include "render/RenderFrame.h"

namespace buffalo::editor
{
// The editor reuses the Runtime image contract; Runtime never includes Editor.
using ViewportFrame = buffalo::RenderFrame;
using PixelFormat = buffalo::PixelFormat;
} // namespace buffalo::editor
