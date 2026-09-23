// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

namespace buffalo::editor
{
// UI maps devices to intentions without platform, camera or renderer types.
enum class InputAction
{
    Orbit,
    Pan,
    Zoom,
    FrameAll,
    NavigationEnd
};

struct InputEvent
{
    InputAction action;
    // Orbit / Pan: fraction of viewport height. Zoom: wheel steps.
    float x = 0.0f;
    float y = 0.0f;
};
} // namespace buffalo::editor
