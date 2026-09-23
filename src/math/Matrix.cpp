#include "math/Matrix.h"

// =======================
// Matrix4
// =======================

Matrix4::Matrix4()
{
    SetIdentity();
}

Matrix4& Matrix4::SetIdentity()
{
    return Set(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

Matrix4 Matrix4::Identity()
{
    Matrix4 result;
    result.SetIdentity();
    return result;
}

Matrix4& Matrix4::Set(
    float m11, float m12, float m13, float m14,
    float m21, float m22, float m23, float m24,
    float m31, float m32, float m33, float m34,
    float m41, float m42, float m43, float m44
)
{
    _11 = m11; _12 = m12; _13 = m13; _14 = m14;
    _21 = m21; _22 = m22; _23 = m23; _24 = m24;
    _31 = m31; _32 = m32; _33 = m33; _34 = m34;
    _41 = m41; _42 = m42; _43 = m43; _44 = m44;

    return *this;
}

Matrix4& Matrix4::SetScale(float sx, float sy, float sz)
{
    return Set(
        sx, 0.0f, 0.0f, 0.0f,
        0.0f, sy, 0.0f, 0.0f,
        0.0f, 0.0f, sz, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

Matrix4 Matrix4::Scale(float sx, float sy, float sz)
{
    Matrix4 result;
    result.SetScale(sx, sy, sz);
    return result;
}

Matrix4& Matrix4::SetTranslation(float tx, float ty, float tz)
{
    return Set(
        1.0f, 0.0f, 0.0f, tx,
        0.0f, 1.0f, 0.0f, ty,
        0.0f, 0.0f, 1.0f, tz,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

Matrix4 Matrix4::Translation(float tx, float ty, float tz)
{
    Matrix4 result;
    result.SetTranslation(tx, ty, tz);
    return result;
}

Matrix4 Matrix4::RotationFromEuler(float rx, float ry, float rz)
{

    float hx = rx * 0.5f;
    float hy = ry * 0.5f;
    float hz = rz * 0.5f;

    float cx = std::cos(hx);
    float sx = std::sin(hx);
    float cy = std::cos(hy);
    float sy = std::sin(hy);
    float cz = std::cos(hz);
    float sz = std::sin(hz);

    float w = cz * cy * cx + sz * sy * sx;
    float x = cz * cy * sx - sz * sy * cx;
    float y = cz * sy * cx + sz * cy * sx;
    float z = sz * cy * cx - cz * sy * sx;

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;

    float xy = x * y;
    float xz = x * z;
    float yz = y * z;

    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    Matrix4 result;

    return result.Set(
        1.0f - 2.0f * (yy + zz), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f,
        2.0f * (xy + wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz - wx), 0.0f,
        2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (xx + yy), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

Matrix4& Matrix4::SetPerspectiveRH(float fovYRadians, float aspect, float nearZ, float farZ)
{
    float tanHalfFOV = std::tan(fovYRadians * 0.5f);

    float yScale = 1.0f / tanHalfFOV;
    float xScale = yScale / aspect;

    float n = nearZ;
    float f = farZ;

    return Set(
        xScale, 0.0f, 0.0f, 0.0f,
        0.0f, yScale, 0.0f, 0.0f,
        0.0f, 0.0f, f / (n - f), (f * n) / (n - f),
        0.0f, 0.0f, -1.0f, 0.0f
    );
}

Matrix4 Matrix4::PerspectiveRH(float fovYRadians, float aspect, float nearZ, float farZ)
{
    Matrix4 result;
    result.SetPerspectiveRH(fovYRadians, aspect, nearZ, farZ);
    return result;
}

Matrix4& Matrix4::SetLookAtRH(const Vector3& eye, const Vector3& target, const Vector3& up)
{

    Vector3 backward = (eye - target).Normalized();             
    Vector3 right = Vector3::Cross(up, backward).Normalized();   
    Vector3 cameraUp = Vector3::Cross(backward, right);

    if (backward.Length() < epsilon)
    {
        backward = Vector3(0.0f, 0.0f, 1.0f);
    }

    if (right.Length() < epsilon)
    {
    
        Vector3 fallbackUp =
            std::abs(backward.y) < 0.999f
            ? Vector3(0.0f, 1.0f, 0.0f)
            : Vector3(1.0f, 0.0f, 0.0f);

        right = Vector3::Cross(fallbackUp, backward).Normalized();
    }

    float tx = Vector3::Dot(right, eye);
    float ty = Vector3::Dot(cameraUp, eye);
    float tz = Vector3::Dot(backward, eye);

    return Set(
        right.x, right.y, right.z, -tx,
        cameraUp.x, cameraUp.y, cameraUp.z, -ty,
        backward.x, backward.y, backward.z, -tz,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

Matrix4 Matrix4::LookAtRH(const Vector3& eye, const Vector3& target, const Vector3& up)
{
    Matrix4 result;
    result.SetLookAtRH(eye, target, up);
    return result;
}

Vector4 Matrix4::operator*(const Vector4& inVector) const
{
    return Vector4(
        _11 * inVector.x + _12 * inVector.y + _13 * inVector.z + _14 * inVector.w,
        _21 * inVector.x + _22 * inVector.y + _23 * inVector.z + _24 * inVector.w,
        _31 * inVector.x + _32 * inVector.y + _33 * inVector.z + _34 * inVector.w,
        _41 * inVector.x + _42 * inVector.y + _43 * inVector.z + _44 * inVector.w
    );
}

Matrix4 Matrix4::operator*(const Matrix4& rhs) const
{
    Matrix4 result;

    result._11 = _11 * rhs._11 + _12 * rhs._21 + _13 * rhs._31 + _14 * rhs._41;
    result._12 = _11 * rhs._12 + _12 * rhs._22 + _13 * rhs._32 + _14 * rhs._42;
    result._13 = _11 * rhs._13 + _12 * rhs._23 + _13 * rhs._33 + _14 * rhs._43;
    result._14 = _11 * rhs._14 + _12 * rhs._24 + _13 * rhs._34 + _14 * rhs._44;

    result._21 = _21 * rhs._11 + _22 * rhs._21 + _23 * rhs._31 + _24 * rhs._41;
    result._22 = _21 * rhs._12 + _22 * rhs._22 + _23 * rhs._32 + _24 * rhs._42;
    result._23 = _21 * rhs._13 + _22 * rhs._23 + _23 * rhs._33 + _24 * rhs._43;
    result._24 = _21 * rhs._14 + _22 * rhs._24 + _23 * rhs._34 + _24 * rhs._44;

    result._31 = _31 * rhs._11 + _32 * rhs._21 + _33 * rhs._31 + _34 * rhs._41;
    result._32 = _31 * rhs._12 + _32 * rhs._22 + _33 * rhs._32 + _34 * rhs._42;
    result._33 = _31 * rhs._13 + _32 * rhs._23 + _33 * rhs._33 + _34 * rhs._43;
    result._34 = _31 * rhs._14 + _32 * rhs._24 + _33 * rhs._34 + _34 * rhs._44;

    result._41 = _41 * rhs._11 + _42 * rhs._21 + _43 * rhs._31 + _44 * rhs._41;
    result._42 = _41 * rhs._12 + _42 * rhs._22 + _43 * rhs._32 + _44 * rhs._42;
    result._43 = _41 * rhs._13 + _42 * rhs._23 + _43 * rhs._33 + _44 * rhs._43;
    result._44 = _41 * rhs._14 + _42 * rhs._24 + _43 * rhs._34 + _44 * rhs._44;

    return result;
}