// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include "math/Vector.h"
#include <vector>
class Texture2D;
struct DirectionalLightParameters
{
    Vector3 directionToLight;
    Vector3 radiance;
};
struct MaterialShaderParameters
{
    const Texture2D* baseColorTexture = nullptr;
    const Texture2D* normalTexture = nullptr;
    Vector3 baseColor{1, 1, 1};
    // Borrowed from Material; valid throughout the synchronous draw.
    const std::vector<DirectionalLightParameters>* lights = nullptr;
    Vector3 tangent;
    Vector3 bitangent;
};
