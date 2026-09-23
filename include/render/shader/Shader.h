#pragma once

#include "render/PipelineTypes.h"

class Shader
{
public:
	virtual ~Shader() = default;

	virtual RasterVertex Vertex(const WorldVertex& input) = 0;

	virtual uint32_t Fragment(const FragmentInput& input) = 0;
};

class BlinnPhongShader : public Shader
{
public:

    RasterVertex Vertex(const WorldVertex& input) override
    {
        RasterVertex output;

        return output;
    }


    uint32_t Fragment(const FragmentInput& input)
    {
        Vector3 N = input.normal.Normalized();

        Vector3 L =
            Vector3(0.4f, 0.8f, 0.3f)
            .Normalized();


        float diffuse =
            Vector3::Dot(N, L);


        if (diffuse < 0)
            diffuse = 0;


        uint8_t c =
            static_cast<uint8_t>(
                diffuse * 255
                );


        return
            (255 << 24) |
            (c << 16) |
            (c << 8) |
            c;
    }
};