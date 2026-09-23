#pragma once

#include "math/Vector.h"

struct WorldVertex
{
    Vector3 position;
    Vector3 normal;
    Vector3 uv;
};

struct ClipVertex
{
    Vector4 position;
    Vector3 normal;
    Vector3 uv;
};

struct RasterVertex
{
    Vector2 screenPosition;
    float depth = 0.0f;

    Vector3 normal;
    Vector3 uv;

    float invW = 0.0f;
};

struct FragmentInput
{
    Vector3 normal;
    Vector3 uv;
    float depth;
};