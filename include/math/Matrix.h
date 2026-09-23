#pragma once

#include "math/Vector.h"

#include <cmath>
#include <cassert>

static constexpr float epsilon = 1e-6f;

class Matrix4
{
public:
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };

        float _data[16];
    };

public:
    Matrix4();

    Matrix4& SetIdentity();

    Matrix4& Set(
        float m11, float m12, float m13, float m14,
        float m21, float m22, float m23, float m24,
        float m31, float m32, float m33, float m34,
        float m41, float m42, float m43, float m44
    );

    Matrix4& SetPerspectiveRH(float fovYRadians, float aspect, float nearZ, float farZ);
    Matrix4& SetLookAtRH(const Vector3& eye, const Vector3& target, const Vector3& up);

    Matrix4& SetScale(float sx, float sy, float sz);
    Matrix4& SetTranslation(float tx, float ty, float tz);

    static Matrix4 Identity();
    static Matrix4 PerspectiveRH(float fovYRadians, float aspect, float nearZ, float farZ);
    static Matrix4 LookAtRH(const Vector3& eye, const Vector3& target, const Vector3& up);
    static Matrix4 Scale(float sx, float sy, float sz);
    static Matrix4 Translation(float tx, float ty, float tz);
    static Matrix4 RotationFromEuler(float rx, float ry, float rz);

    Vector4 operator*(const Vector4& inVector) const;
    Matrix4 operator*(const Matrix4& rhs) const;
};