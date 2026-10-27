// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once

#include "scene/ObjectId.h"
#include <string>
#include "math/Vector.h"
#include "math/Matrix.h"

struct Camera
{
    Camera();
    Camera(const Camera& other);
    Camera& operator=(const Camera& other);
    ObjectId Id() const
    {
        return id_;
    }
    std::string name = "Camera";

    void UpdateMatrices();

    float FrameSphere(
        const Vector3& center,
        float radius,
        const Vector3& backward,
        float viewportAspect,
        float padding = 1.15f);

    Matrix4 viewMatrix;
    Matrix4 projectionMatrix;

    float fovYRadians = 1.047197551f; // 60 degrees
    float aspect = 1.77f;
    bool autoAspect = true;
    float nearZ = 0.1f;
    float farZ = 1000.0f;

    Vector3 position = Vector3(0.0f, 0.0f, 3.0f);
    Vector3 target = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);

private:
    const ObjectId id_ = NewObjectId();
};
