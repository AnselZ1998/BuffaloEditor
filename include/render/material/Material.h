// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#pragma once
#include "render/shader/Shader.h"
#include <memory>
#include <string>
enum class ShaderKind
{
    BlinnPhong,
    Unlit,
    Normal
};
class Material
{
public:
    Material();
    Shader* GetShader()
    {
        return shader_.get();
    }
    ShaderKind GetShaderKind() const
    {
        return kind_;
    }
    void SetShader(ShaderKind kind);
    void SetBaseColorTexture(std::shared_ptr<Texture2D> texture);
    const Texture2D* GetBaseColorTexture() const;
    bool LoadTexture(const std::string& path, bool normal);
    const std::string& TexturePath(bool normal) const;
    std::uint64_t TextureRevision() const
    {
        return textureRevision_;
    }
    MaterialShaderParameters GetShaderParameters() const;
    Vector3 baseColor{1, 1, 1};
    std::vector<DirectionalLightParameters> lights;

private:
    ShaderKind kind_ = ShaderKind::BlinnPhong;
    std::unique_ptr<Shader> shader_;
    std::shared_ptr<Texture2D> baseColorTexture_;
    std::shared_ptr<Texture2D> normalTexture_;
    std::string baseColorPath_;
    std::string normalPath_;
    std::uint64_t textureRevision_ = 1;
};
