#version 330 core

layout(location = 0) in vec3 localPosition;
layout(location = 1) in vec3 localNormal;

uniform mat4 model;
uniform mat4 viewProjection;

out vec3 worldNormal;

void main()
{
    gl_Position = viewProjection * model * vec4(localPosition, 1.0);
    // Buffalo's RH projection uses [0, 1] depth; OpenGL 3.3 uses [-1, 1].
    gl_Position.z = 2.0 * gl_Position.z - gl_Position.w;
    worldNormal = transpose(inverse(mat3(model))) * localNormal;
}
