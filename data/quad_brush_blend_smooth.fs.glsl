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

    float newValue = baseValue;
    if (influence > 0)
    {
        float L = 4.0f / 2048.0f;
        float accumulated = baseValue;
        int samples = 1;

        int radialStepCount = 24;
        int outwardStepCount = 12;

        for (int i = 0; i < radialStepCount; i++)
        {
            float theta = (6.283f * i) / radialStepCount;
            vec2 D = vec2(sin(theta), cos(theta));
            
            for (int j = 1; j <= outwardStepCount; j++)
            {
                vec2 offset = D * (L * j);
                float sampleInfluence = texture(influenceTexture, uv + offset).r;
                if (sampleInfluence > 0)
                {
                    accumulated += texture(baseTexture, uv + offset).r;
                    samples++;
                }
                else
                {
                    break;
                }
            }
        }

        newValue = mix(baseValue, accumulated / samples, influence);
    }
    
    FragColor = vec4(newValue, newValue, newValue, 1);
}