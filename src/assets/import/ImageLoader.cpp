#include "assets/import/ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ImageData ImageLoader::LoadRGBA8(const std::string& path)
{
    ImageData image;

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* data = stbi_load(
        path.c_str(),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha
    );

    if (data == nullptr)
    {
        return image;
    }

    const std::size_t pixelCount =
        static_cast<std::size_t>(width) *
        static_cast<std::size_t>(height);

    image.width = width;
    image.height = height;
    image.pixels.resize(pixelCount);

    for (std::size_t i = 0; i < pixelCount; ++i)
    {
        const std::uint32_t r = data[i * 4 + 0];
        const std::uint32_t g = data[i * 4 + 1];
        const std::uint32_t b = data[i * 4 + 2];
        const std::uint32_t a = data[i * 4 + 3];

        image.pixels[i] =
            r |
            (g << 8) |
            (b << 16) |
            (a << 24);
    }

    stbi_image_free(data);

    return image;
}