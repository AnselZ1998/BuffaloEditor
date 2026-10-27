#include "render/texture/Texture2D.h"
#include "assets/import/ImageLoader.h"
#include "assets/ImageData.h"

#include <filesystem>
#include <algorithm>

bool Texture2D::LoadFromFile(const std::string& path, TextureFormat format)
{
    if (!std::filesystem::exists(path))
    {
        return false;
    }

    ImageData pixelsData;

    switch (format)
    {
    case TextureFormat::RGBA8:
        pixelsData = ImageLoader::LoadRGBA8(path);
        break;

    default:
        return false;
    }

    if (!pixelsData.IsValid())
    {
        return false;
    }

    mWidth = pixelsData.width;
    mHeight = pixelsData.height;
    mFormat = format;
    mPixels = std::move(pixelsData.pixels);

    return true;
}

int Texture2D::GetWidth() const
{
    return mWidth;
}

int Texture2D::GetHeight() const
{
    return mHeight;
}

TextureFormat Texture2D::GetFormat() const
{
    return mFormat;
}

bool Texture2D::IsValid() const
{
    return mWidth > 0 &&
        mHeight > 0 &&
        mPixels.size() ==
            static_cast<std::size_t>(mWidth) *
            static_cast<std::size_t>(mHeight);
}

const std::uint32_t* Texture2D::Data() const
{
    if (mPixels.empty())
    {
        return nullptr;
    }

    return mPixels.data();
}

std::uint32_t Texture2D::Sample(const Vector3& uv) const
{
    if (!IsValid())
    {
        return 0xFFFF00FF;
    }

    float u = std::clamp(uv.x, 0.0f, 1.0f);
    float v = std::clamp(uv.y, 0.0f, 1.0f);

    int x = static_cast<int>(u * (mWidth - 1));
    int y = static_cast<int>((1.0f - v) * (mHeight - 1));

    return mPixels[y * mWidth + x];
}