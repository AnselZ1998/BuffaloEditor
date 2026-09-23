#pragma once

#include "math/Vector.h"

class Quaternion
{
public:
	union
	{
		struct
		{
			float w, x, y, z;
		};
		float data[4];
	};
    Quaternion()
        : w(1.0f), x(0.0f), y(0.0f), z(0.0f)
    {
    }

    Quaternion(float inW, float inX, float inY, float inZ)
        : w(inW), x(inX), y(inY), z(inZ)
    {
    }

    explicit Quaternion(const Vector4& inVector)
        : w(inVector.w),
        x(inVector.x),
        y(inVector.y),
        z(inVector.z)
    {
    }

    Quaternion(const Quaternion& other) = default;
    Quaternion& operator=(const Quaternion& other) = default;

    Quaternion& SetFromEuler(const Vector3& inEuler);
};

