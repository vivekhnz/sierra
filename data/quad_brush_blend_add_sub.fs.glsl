#version 430 core
layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D baseTexture;
layout(binding = 1) uniform sampler2D influenceTexture;

uniform float blendSign;

out vec4 FragColor;

void main()
{
    float baseValue = texture(baseTexture, uv).r;
    float influence = texture(influenceTexture, uv).r;

    float newValue = baseValue + (influence * blendSign);

    FragColor = vec4(newValue, newValue, newValue, 1);
}