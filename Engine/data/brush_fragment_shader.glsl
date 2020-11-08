#version 430 core
layout(location = 0) in vec2 uv;

out vec4 FragColor;

void main()
{
    float innerRadius = 0.1f;
    float outerRadius = 1.0f;
    float strength = 0.02f;

    float value = distance(uv, vec2(0.5f)) * 2.0f;
    value = (-value + outerRadius) / (outerRadius - innerRadius);
    value = max(min(value, 1.0f), 0.0f);
    value = value * strength;
    
    FragColor = vec4(1.0f, 1.0f, 1.0f, value);
}