#pragma once

#include <cstdint>
#include <vector>
#include <cstddef>

struct ImageData
{
    int width = 0;
    int height = 0;

    std::vector<std::uint32_t> pixels;

    bool IsValid() const
    {
        return 
        width > 0 &&
        height > 0 &&
        pixels.size() == (static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    }
};