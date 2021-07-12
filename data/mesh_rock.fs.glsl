#version 430 core

layout(location = 1) in vec3 in_normal;

layout (std140, binding = 1) uniform Lighting
{
    vec4 lighting_lightDir;
    bool lighting_isEnabled;
    bool lighting_isTextureEnabled;
    bool lighting_isNormalMapEnabled;
    bool lighting_isAOMapEnabled;
    bool lighting_isDisplacementMapEnabled;
};

out vec4 FragColor;

void main()
{
    float ambientLight = 0.15f;
    float nDotL = dot(in_normal, lighting_lightDir.xyz);
    float lightingAmplitude = lighting_isEnabled
        ? ambientLight + pow(0.5 + (nDotL * 0.5), 2)
        : 1.0f;
    vec3 albedo = vec3(0.72, 0.69, 0.65);
    vec3 color = albedo * lightingAmplitude;

    FragColor = vec4(color, 1);
}