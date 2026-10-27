#version 330 core
// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT
in vec3 worldNormal;
in vec3 worldPosition;
in vec2 uv;
out vec4 color;
uniform int lightCount;
uniform vec3 lightDirections[64];
uniform vec3 lightRadiance[64];
uniform vec3 baseColor;
uniform sampler2D baseTexture;
uniform sampler2D normalTexture;
uniform bool hasBaseTexture;
uniform bool hasNormalTexture;
uniform int shaderKind;
void main()
{
    vec3 n = normalize(worldNormal);
    vec2 textureUV = vec2(uv.x, 1.0 - uv.y);
    if (hasNormalTexture)
    {
        vec3 dp1 = dFdx(worldPosition);
        vec3 dp2 = dFdy(worldPosition);
        vec2 duv1 = dFdx(uv);
        vec2 duv2 = dFdy(uv);
        float determinant = duv1.x * duv2.y - duv1.y * duv2.x;
        if (abs(determinant) > 0.0000001)
        {
            vec3 tangent = (dp1 * duv2.y - dp2 * duv1.y) / determinant;
            vec3 bitangent = (dp2 * duv1.x - dp1 * duv2.x) / determinant;
            vec3 t = normalize(tangent - n * dot(n, tangent));
            vec3 b = cross(n, t);
            if (dot(b, bitangent) < 0.0) { b = -b; }
            n = normalize(mat3(t, b, n) * (texture(normalTexture, textureUV).rgb * 2.0 - 1.0));
        }
    }
    vec3 base = baseColor;
    if (hasBaseTexture) { base *= texture(baseTexture, textureUV).rgb; }
    vec3 lighting = vec3(0.0);
    for (int i = 0; i < lightCount; ++i) { lighting += lightRadiance[i] * max(dot(n, lightDirections[i]), 0.0); }
    color = vec4(shaderKind == 1 ? base : (shaderKind == 2 ? n * 0.5 + 0.5 : base * lighting), 1.0);
}
