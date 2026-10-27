// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "render/material/Material.h"
Material::Material()
{
    SetShader(ShaderKind::BlinnPhong);
}
void Material::SetShader(ShaderKind kind)
{
    kind_ = kind;
    switch (kind)
    {
    case ShaderKind::BlinnPhong:
        shader_ = std::make_unique<BlinnPhongShader>();
        break;
    case ShaderKind::Unlit:
        shader_ = std::make_unique<UnlitShader>();
        break;
    case ShaderKind::Normal:
        shader_ = std::make_unique<NormalShader>();
        break;
    }
}
void Material::SetBaseColorTexture(std::shared_ptr<Texture2D> texture)
{
    baseColorTexture_ = std::move(texture);
    ++textureRevision_;
}
const Texture2D* Material::GetBaseColorTexture() const
{
    return baseColorTexture_.get();
}
bool Material::LoadTexture(const std::string& path, bool normal)
{
    std::shared_ptr<Texture2D> candidate;
    if (!path.empty())
    {
        candidate = std::make_shared<Texture2D>();
        if (!candidate->LoadFromFile(path))
        {
            return false;
        }
    }
    (normal ? normalTexture_ : baseColorTexture_) = std::move(candidate);
    (normal ? normalPath_ : baseColorPath_) = path;
    ++textureRevision_;
    return true;
}
const std::string& Material::TexturePath(bool normal) const
{
    return normal ? normalPath_ : baseColorPath_;
}
MaterialShaderParameters Material::GetShaderParameters() const
{
    MaterialShaderParameters params;
    params.baseColorTexture = baseColorTexture_.get();
    params.normalTexture = normalTexture_.get();
    params.baseColor = baseColor;
    params.lights = &lights;
    return params;
}
