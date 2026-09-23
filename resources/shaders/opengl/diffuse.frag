#version 330 core

in vec3 worldNormal;
out vec4 color;

void main()
{
    // Same first lighting exercise as the CPU shader. PBR/material bindings
    // can be added here without putting GPU programs into Scene or Model.
    vec3 lightDirection = normalize(vec3(0.4, 0.8, 0.3));
    float diffuse = max(dot(normalize(worldNormal), lightDirection), 0.0);
    color = vec4(vec3(diffuse), 1.0);
}
