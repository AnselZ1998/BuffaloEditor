#pragma once
#include "render/shader/Shader.h"
#include <memory>

class Material
{
public:
    Material()
    {
        mShader = std::make_unique<BlinnPhongShader>();
    }

    Shader* GetShader()
    {
        return mShader.get();
    }

private:
    std::unique_ptr<Shader> mShader;
};
