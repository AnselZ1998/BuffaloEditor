// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include "scene/light/ILight.h"
class DirectionalLight final : public ILight
{
public:
    DirectionalLight()
    {
        name = "Directional Light";
    }
    LightKind Kind() const override
    {
        return LightKind::Directional;
    }
};
