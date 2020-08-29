#version 430 core
layout(location = 0) in vec3 vertexNormal;
layout(location = 1) in vec2 texcoord;

layout (std140, binding = 1) uniform Lighting
{
    vec4 lighting_lightDir;
    bool lighting_isEnabled;
    bool lighting_isTextureEnabled;
    bool lighting_isNormalMapEnabled;
    bool lighting_isAOMapEnabled;
    bool lighting_isDisplacementMapEnabled;
    bool lighting_isRoughnessMapEnabled;
};

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D aoTexture;
uniform sampler2D roughnessTexture;

out vec4 FragColor;

void main()
{
    float roughness = texture(roughnessTexture, texcoord).r;
    vec3 texNormal = lighting_isNormalMapEnabled
        ? (texture(normalTexture, texcoord).rgb * 2.0f) - 1.0f
        : vec3(0.0f);
    vec3 normal = normalize(vertexNormal - (texNormal * 0.5f));
    normal = lighting_isRoughnessMapEnabled ? normalize(normal - vec3(0, roughness * 0.8f, 0)) : normal;
    float ambientLight = 0.2f;
    float nDotL = dot(normal, lighting_lightDir.xyz);
    float lightingCol = lighting_isEnabled ? max(nDotL, ambientLight) : 1.0f;
    vec3 albedo = lighting_isTextureEnabled ? texture(albedoTexture, texcoord).rgb : vec3(1.0f);
    float ao = lighting_isAOMapEnabled ? mix(0.6f, 1.0f, texture(aoTexture, texcoord).r) : 1.0f;
    FragColor = vec4(albedo * lightingCol * ao, 1.0f);
}