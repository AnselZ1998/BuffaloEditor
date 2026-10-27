// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include "render/PipelineTypes.h"
#include "render/material/MaterialShaderParameters.h"
#include "render/texture/Texture2D.h"
#include <algorithm>
class Shader
{
public:
    virtual ~Shader() = default;
    virtual RasterVertex Vertex(const WorldVertex& input) = 0;
    virtual uint32_t Fragment(const FragmentInput& input, const MaterialShaderParameters& params) = 0;

protected:
    static Vector3 BaseColor(const FragmentInput& input, const MaterialShaderParameters& params)
    {
        if (!params.baseColorTexture)
        {
            return params.baseColor;
        }
        const auto c = params.baseColorTexture->Sample(input.uv);
        return {params.baseColor.x * (c & 255) / 255.0f, params.baseColor.y * ((c >> 8) & 255) / 255.0f, params.baseColor.z * ((c >> 16) & 255) / 255.0f};
    }
    static Vector3 Normal(const FragmentInput& input, const MaterialShaderParameters& params)
    {
        const Vector3 n = input.normal.Normalized();
        if (!params.normalTexture || params.tangent.Length() < 1e-6f)
        {
            return n;
        }
        const auto c = params.normalTexture->Sample(input.uv);
        const Vector3 mapped((c & 255) / 127.5f - 1, ((c >> 8) & 255) / 127.5f - 1, ((c >> 16) & 255) / 127.5f - 1);
        const Vector3 t = (params.tangent - n * Vector3::Dot(n, params.tangent)).Normalized();
        Vector3 b = Vector3::Cross(n, t);
        if (Vector3::Dot(b, params.bitangent) < 0)
        {
            b = b * -1.0f;
        }
        return (t * mapped.x + b * mapped.y + n * mapped.z).Normalized();
    }
    static std::uint32_t Pack(const Vector3& color)
    {
        const auto channel = [](float v)
        {
            return static_cast<std::uint32_t>(std::clamp(v, 0.0f, 1.0f) * 255.0f);
        };
        return 0xFF000000u | (channel(color.x) << 16) | (channel(color.y) << 8) | channel(color.z);
    }
};
class BlinnPhongShader final : public Shader
{
public:
    RasterVertex Vertex(const WorldVertex&) override
    {
        return {};
    }
    uint32_t Fragment(const FragmentInput& input, const MaterialShaderParameters& params) override
    {
        const Vector3 n = Normal(input, params);
        Vector3 lighting;
        if (params.lights)
        {
            for (const auto& light : *params.lights)
            {
                lighting = lighting + light.radiance * std::max(0.0f, Vector3::Dot(n, light.directionToLight));
            }
        }
        const Vector3 base = BaseColor(input, params);
        return Pack({base.x * lighting.x, base.y * lighting.y, base.z * lighting.z});
    }
};
class UnlitShader final : public Shader
{
public:
    RasterVertex Vertex(const WorldVertex&) override
    {
        return {};
    }
    uint32_t Fragment(const FragmentInput& input, const MaterialShaderParameters& params) override
    {
        return Pack(BaseColor(input, params));
    }
};
class NormalShader final : public Shader
{
public:
    RasterVertex Vertex(const WorldVertex&) override
    {
        return {};
    }
    uint32_t Fragment(const FragmentInput& input, const MaterialShaderParameters& params) override
    {
        return Pack((Normal(input, params) + Vector3(1, 1, 1)) * 0.5f);
    }
};
