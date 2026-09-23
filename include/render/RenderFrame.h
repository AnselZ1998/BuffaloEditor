// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <limits>

namespace buffalo
{
enum class PixelFormat
{
    BGRA8,
    RGBA8
};

enum class FrameStorage
{
    CpuPixels,
    OpenGLTexture
};

// Borrowed output, valid until the next Render(), resize, backend replacement
// or shutdown. Rendering and presentation currently run on the same thread.
// CPU pixels are top-to-bottom BGRA8. GPU output is an RGBA8 GL_TEXTURE_2D,
// bottom-to-top, already complete when Render returns. GPU presenters must
// belong to the same graphicsDevice (OpenGL context share group).
struct RenderFrame
{
    const std::uint32_t* pixels = nullptr;
    int width = 0;
    int height = 0;
    int rowPitchBytes = 0;
    PixelFormat format = PixelFormat::BGRA8;

    FrameStorage storage = FrameStorage::CpuPixels;
    std::uint32_t textureId = 0;
    const void* graphicsDevice = nullptr;

    bool IsValid() const
    {
        if (storage == FrameStorage::OpenGLTexture)
        {
            return textureId != 0 && graphicsDevice && width > 0 && height > 0 && format == PixelFormat::RGBA8;
        }
        return storage == FrameStorage::CpuPixels && format == PixelFormat::BGRA8 && pixels &&
               width > 0 && height > 0 && width <= std::numeric_limits<int>::max() / 4 && rowPitchBytes >= width * 4;
    }
};
} // namespace buffalo
