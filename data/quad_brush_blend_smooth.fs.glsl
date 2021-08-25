#version 430 core
layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D baseTexture;
layout(binding = 1) uniform sampler2D influenceTexture;

uniform int iterationCount;
uniform int iteration;
uniform int heightmapWidth;
uniform vec2 blurDirection;

out vec4 FragColor;

float getHeight(vec2 uv, float defaultValue)
{
    float influence = texture(influenceTexture, uv).r;
    return influence == 0
        ? defaultValue
        : texture(baseTexture, uv).r;
}

void main()
{
    float baseValue = texture(baseTexture, in_uv).r;
    float newValue = baseValue;

    float influence = texture(influenceTexture, in_uv).r;
    influence = clamp((influence * iterationCount) - iteration, 0, 1);
    if (influence > 0)
    {
        // 9-tap Gaussian blur utilising linear sampling
        // see: https://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
        float sampleStepLength = 12.0f;
        vec2 blurOffset = blurDirection * (sampleStepLength / heightmapWidth);
        float centerValue = texture(baseTexture, in_uv).r;
        float blurred = 0;

        vec2 offsetInner = blurOffset * 1.3846153846f;
        vec2 offsetOuter = blurOffset * 3.2307692308f;
        float weightInner = 0.3162162162f;
        float weightOuter = 0.0702702703f;
        float weightCenter = 0.2270270270f;

        blurred += getHeight(in_uv - offsetOuter, centerValue) * weightOuter;
        blurred += getHeight(in_uv - offsetInner, centerValue) * weightInner;
        blurred += centerValue * weightCenter;
        blurred += getHeight(in_uv + offsetInner, centerValue) * weightInner;
        blurred += getHeight(in_uv + offsetOuter, centerValue) * weightOuter;

        newValue = mix(baseValue, blurred, influence);
    }
    FragColor = vec4(newValue, newValue, newValue, 1);
}