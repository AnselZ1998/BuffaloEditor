#pragma once

#include "math/Vector.h"

struct Bounds2
{
    Vector2 minPoint; 
    Vector2 maxPoint; 

    Bounds2(const Vector2& minP, const Vector2& maxP)
        : minPoint(minP), maxPoint(maxP)
    {
    }
};