#version 410 core
layout(location = 0) in vec3 vertexNormal;
layout(location = 1) in vec2 texcoord;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform bool isLightingEnabled;
uniform bool isTextureEnabled;
uniform bool isNormalMapEnabled;

out vec4 FragColor;

void main()
{
    vec3 texNormal = isNormalMapEnabled
        ? (texture(normalTexture, texcoord).rgb * 2.0f) - 1.0f
        : vec3(0.0f);
    vec3 normal = normalize(vertexNormal - (texNormal * 0.5f));
    vec3 lightDir = vec3(0.7f, 0.3f, 0.2f);
    float ambientLight = 0.2f;
    float nDotL = dot(normal, lightDir);
    float lightingCol = isLightingEnabled ? max(nDotL, ambientLight) : 1.0f;
    vec3 albedo = isTextureEnabled ? texture(albedoTexture, texcoord).rgb : vec3(1.0f);
    FragColor = vec4(albedo * lightingCol, 1.0f);
}