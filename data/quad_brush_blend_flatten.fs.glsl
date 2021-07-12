#version 430 core
layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D baseTexture;
layout(binding = 1) uniform sampler2D influenceTexture;

uniform float flattenHeight;

out vec4 FragColor;

void main()
{
    float baseValue = texture(baseTexture, uv).r;
    float influence = texture(influenceTexture, uv).r;
    
    float newValue = mix(baseValue, flattenHeight, 1 - pow(1 - influence, 2));

    FragColor = vec4(newValue, newValue, newValue, 1);
}