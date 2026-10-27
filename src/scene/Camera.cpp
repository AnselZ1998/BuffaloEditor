// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "scene/Camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera(const Camera& other)
    : Camera()
{
    *this = other;
}

Camera& Camera::operator=(const Camera& other)
{
    // Copy settings, never identity.
    name = other.name;
    position = other.position;
    target = other.target;
    up = other.up;
    fovYRadians = other.fovYRadians;
    aspect = other.aspect;
    autoAspect = other.autoAspect;
    nearZ = other.nearZ;
    farZ = other.farZ;
    UpdateMatrices();
    return *this;
}

Camera::Camera()
{
    UpdateMatrices();
}

float Camera::FrameSphere(
    const Vector3& center,
    float radius,
    const Vector3& backward,
    float viewportAspect,
    float padding)
{
    radius = std::max(radius, 0.1f);
    aspect = std::max(viewportAspect, 0.001f);

    const float verticalHalfAngle = fovYRadians * 0.5f;
    const float horizontalHalfAngle = std::atan(std::tan(verticalHalfAngle) * aspect);
    const float limitingHalfAngle = std::min(verticalHalfAngle, horizontalHalfAngle);

    const float distance = radius / std::sin(limitingHalfAngle) * padding;

    target = center;
    position = center + backward * distance;
    up = Vector3(0.0f, 1.0f, 0.0f);

    nearZ = std::max(0.01f, radius * 0.01f);
    farZ = std::max(1000.0f, radius * 100.0f);

    UpdateMatrices();

    return distance;
}

void Camera::UpdateMatrices()
{
    viewMatrix = Matrix4::LookAtRH(
        position,
        target,
        up);

    projectionMatrix = Matrix4::PerspectiveRH(
        fovYRadians,
        aspect,
        nearZ,
        farZ);
}
