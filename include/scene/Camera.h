#pragma once

#include "math/Vector.h"
#include "math/Matrix.h"

struct Camera
{
    Camera();

    void UpdateMatrices();

    float FrameSphere(
        const Vector3& center,
        float radius,
        const Vector3& backward,
        float viewportAspect,
        float padding = 1.15f
    );

    Matrix4 viewMatrix;
    Matrix4 projectionMatrix;

    float fovYRadians = 1.047197551f; // 60 degrees
    float aspect = 1.77f;
    float nearZ = 0.1f;
    float farZ = 1000.0f;

    Vector3 position = Vector3(0.0f, 0.0f, 3.0f);
    Vector3 target = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
};