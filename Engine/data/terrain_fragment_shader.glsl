#version 430 core
layout(location = 0) in vec3 vertexNormal;
layout(location = 1) in vec2 texcoord;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D aoTexture;
uniform sampler2D roughnessTexture;
uniform bool isLightingEnabled;
uniform bool isTextureEnabled;
uniform bool isNormalMapEnabled;
uniform bool isAOMapEnabled;
uniform bool isRoughnessMapEnabled;
uniform vec3 lightDir;

out vec4 FragColor;

void main()
{
    float roughness = texture(roughnessTexture, texcoord).r;
    vec3 texNormal = isNormalMapEnabled
        ? (texture(normalTexture, texcoord).rgb * 2.0f) - 1.0f
        : vec3(0.0f);
    vec3 normal = normalize(vertexNormal - (texNormal * 0.5f));
    normal = isRoughnessMapEnabled ? normalize(normal - vec3(0, roughness * 0.8f, 0)) : normal;
    float ambientLight = 0.2f;
    float nDotL = dot(normal, lightDir);
    float lightingCol = isLightingEnabled ? max(nDotL, ambientLight) : 1.0f;
    vec3 albedo = isTextureEnabled ? texture(albedoTexture, texcoord).rgb : vec3(1.0f);
    float ao = isAOMapEnabled ? mix(0.6f, 1.0f, texture(aoTexture, texcoord).r) : 1.0f;
    FragColor = vec4(albedo * lightingCol * ao, 1.0f);
}