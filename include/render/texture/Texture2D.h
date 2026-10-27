#pragma once

#include "math/Vector.h"

#include <cstdint>
#include <vector>
#include <string>

enum class TextureFormat
{
    R8,
    RGB8,
    RGBA8,
};

class Texture2D
{
public:
    Texture2D() = default;
    ~Texture2D() = default;

    bool LoadFromFile(const std::string& path, TextureFormat format = TextureFormat::RGBA8);

    int GetWidth() const;
    int GetHeight() const;

    TextureFormat GetFormat() const;

    bool IsValid() const;

    const std::uint32_t* Data() const;

    std::uint32_t Sample(const Vector3& uv) const;

private:
    int mWidth = 0;
    int mHeight = 0;

    TextureFormat mFormat = TextureFormat::RGBA8;

    std::vector<std::uint32_t> mPixels;
};