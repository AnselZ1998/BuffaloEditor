#pragma once

#include "assets/ImageData.h"

#include <string>

class ImageLoader
{
public:
    static ImageData LoadRGBA8(const std::string& path);
};