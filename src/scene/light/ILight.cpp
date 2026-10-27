// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "scene/light/ILight.h"
#include <cmath>
void ILight::UpdateOrbit(double elapsedSeconds)
{
    if (!orbit.enabled || !std::isfinite(elapsedSeconds) || elapsedSeconds < 0)
    {
        return;
    }
    orbit.angleDegrees = static_cast<float>(std::fmod(orbit.angleDegrees + elapsedSeconds * orbit.speedDegrees, 360.0));
    const float angle = orbit.angleDegrees * 0.01745329252f;
    position = orbit.target + Vector3(std::cos(angle) * orbit.radius, orbit.height, std::sin(angle) * orbit.radius);
    const Vector3 delta = orbit.target - position;
    if (delta.Length() > 1e-6f)
    {
        direction = delta.Normalized();
    }
}
