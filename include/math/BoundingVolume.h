#pragma once

#include "math/Vector.h"
#include <algorithm>

class BoundingVolume
{
public:
    float minX;
    float minY;
    float maxX;
    float maxY;

    void GetBoundingVolume(
        const Vector4& inVectorPa,
        const Vector4& inVectorPb,
        const Vector4& inVectorPc
    );
};

struct Bounds3
{
    Vector3 min{0.0f, 0.0f, 0.0f};
    Vector3 max{0.0f, 0.0f, 0.0f};

    bool valid = false;

    void Expand(const Vector3& point)
    {

        if (!valid)
        {
            min = point;
            max = point;
            valid = true;
            return;
        }

        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);

        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    Vector3 Center() const
    {
        if (!valid)
        {
            return Vector3(0.0f, 0.0f, 0.0f);
        }

        return (min + max) * 0.5f;
    }

    float Radius() const
    {
        if (!valid)
        {
            return 0.0f;
        }

        return (max - min).Length() * 0.5f;
    }
};