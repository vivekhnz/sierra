#version 430 core
layout(location = 0) in vec2 uv;

uniform float brushFalloff;
uniform float brushStrength;

out vec4 FragColor;

void main()
{
    float d = 1 - min(distance(uv, vec2(0.5f)) * 2, 1);
    float value = (cos(d * 3.14f) * 0.5f) + 0.5f;
    value = (1 - value) / (1 - brushFalloff);
    value = max(min(value, 1.0f), 0.0f);
    value = value * brushStrength;
    FragColor = vec4(value, 0, 0, 0);
}