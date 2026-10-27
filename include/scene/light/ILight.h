// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include "math/Vector.h"
#include "scene/ObjectId.h"
#include <string>
enum class LightKind
{
    Directional
};
struct LightOrbit
{
    bool enabled = false;
    Vector3 target{0, 0, 0};
    float radius = 3.0f;
    float height = 2.0f;
    float speedDegrees = 30.0f;
    float angleDegrees = 0.0f;
};
class ILight
{
public:
    ILight() = default;
    ILight(const ILight&) = delete;
    ILight& operator=(const ILight&) = delete;
    virtual ~ILight() = default;
    virtual LightKind Kind() const = 0;
    ObjectId Id() const
    {
        return id_;
    }
    void UpdateOrbit(double elapsedSeconds);
    std::string name = "Light";
    bool enabled = true;
    Vector3 position{3, 2, 0};
    // Direction in which light travels; shaders use its negative.
    Vector3 direction = Vector3(-0.4f, -0.8f, -0.3f).Normalized();
    Vector3 color{1, 1, 1};
    float intensity = 1.0f;
    LightOrbit orbit;

private:
    const ObjectId id_ = NewObjectId();
};
