#version 330 core
// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT
layout(location = 0) in vec3 localPosition;
layout(location = 1) in vec3 localNormal;
layout(location = 2) in vec2 localUV;
uniform mat4 model;
uniform mat4 viewProjection;
out vec3 worldNormal;
out vec3 worldPosition;
out vec2 uv;
void main()
{
    vec4 world = model * vec4(localPosition, 1.0);
    gl_Position = viewProjection * world;
    gl_Position.z = 2.0 * gl_Position.z - gl_Position.w;
    worldPosition = world.xyz;
    worldNormal = transpose(inverse(mat3(model))) * localNormal;
    uv = localUV;
}
