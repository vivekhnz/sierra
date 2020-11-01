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
uniform vec2 textureScale;
uniform vec2 brushHighlightPos;
uniform float brushHighlightStrength;

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

    float r = 0.03f;
    
    // outline thickness is based on depth
    float w = min(gl_FragCoord.w, 0.028f);
    float o = 0.09f - (5.09f * w) + (88.16f * pow(w, 2));
    
    float d = distance(texcoord / textureScale, brushHighlightPos) / r;
    float a = clamp(1 - d, 0, 1) + clamp(1 - abs((d - 1) / o), 0, 1);
    vec3 brushHighlight = vec3(0.0f, 1.0f, 0.25f) * a;

    FragColor = vec4((albedo * lightingCol * ao) + (brushHighlight * brushHighlightStrength), 1.0f);
}