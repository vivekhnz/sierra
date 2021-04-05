#version 430 core
layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D baseTexture;
layout(binding = 1) uniform sampler2D influenceTexture;

uniform int iterationCount;
uniform int iteration;

out vec4 FragColor;

void main()
{
    float baseValue = texture(baseTexture, uv).r;
    float influence = texture(influenceTexture, uv).r;

    influence = clamp((influence * iterationCount) - iteration, 0, 1);
    
    float offsetLength = 64.0f / 2048.0f;
    float blurredValue = baseValue;
    int radialStepCount = 12;
    for (int i = 0; i < radialStepCount; i++)
    {
        float x = (6.283f * i) / radialStepCount;
        vec2 offset = vec2(sin(x), cos(x)) * offsetLength;
        blurredValue += texture(baseTexture, uv + offset).r;
    }
    blurredValue /= radialStepCount + 1.0f;

    float newValue = mix(baseValue, blurredValue, influence);

    FragColor = vec4(newValue, newValue, newValue, 1);
}