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
uniform float brushHighlightRadius;
uniform float brushHighlightFalloff;

out vec4 FragColor;

vec3 calcBrushHighlight()
{
    float highlightRadius = brushHighlightRadius * 0.5f;
    float distFromHighlight = distance(texcoord / textureScale, brushHighlightPos);
    
    // outline thickness is based on depth
    float outlineWidth = max(min(0.003 - (0.07 * gl_FragCoord.w), 0.003), 0.0005);
    float outlineIntensity =
        max(1 - abs((((distFromHighlight - highlightRadius) / outlineWidth) * 2) - 1), 0);
    float influenceAreaIntensity = (
        (distFromHighlight / highlightRadius) - brushHighlightFalloff)
        / (1 - brushHighlightFalloff);
    influenceAreaIntensity = 1 - clamp(influenceAreaIntensity, 0, 1);

    float highlightIntensity = influenceAreaIntensity + outlineIntensity;
    return vec3(0.0f, 1.0f, 0.25f) * highlightIntensity;
}

void main()
{
    // calculate normal
    float roughness = texture(roughnessTexture, texcoord).r;
    vec3 texNormal = lighting_isNormalMapEnabled
        ? (texture(normalTexture, texcoord).rgb * 2.0f) - 1.0f
        : vec3(0.0f);
    vec3 normal = normalize(vertexNormal - (texNormal * 0.5f));
    normal = lighting_isRoughnessMapEnabled
        ? normalize(normal - vec3(0, roughness * 0.8f, 0))
        : normal;

    // sample albedo texture
    vec3 albedo = lighting_isTextureEnabled
        ? texture(albedoTexture, texcoord).rgb
        : vec3(1.0f);

    // calculate lighting
    float ambientLight = 0.3f;
    float nDotL = dot(normal, lighting_lightDir.xyz);
    float lightingAmplitude = lighting_isEnabled
        ? pow(max(0.5 + (nDotL * 0.5), ambientLight), 4)
        : 1.0f;

    // sample ambient occlusion texture
    float ao = lighting_isAOMapEnabled
        ? mix(0.6f, 1.0f, texture(aoTexture, texcoord).r)
        : 1.0f;

    // calculate final fragment color
    vec3 terrainColor = albedo * lightingAmplitude * ao;
    vec3 brushHighlight = calcBrushHighlight();
    FragColor = vec4(terrainColor + (brushHighlight * brushHighlightStrength), 1.0f);
}