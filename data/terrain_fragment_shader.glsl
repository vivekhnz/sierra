#version 430 core
layout(location = 0) in vec3 vertexNormal;
layout(location = 1) in vec3 texcoord;

layout (std140, binding = 1) uniform Lighting
{
    vec4 lighting_lightDir;
    bool lighting_isEnabled;
    bool lighting_isTextureEnabled;
    bool lighting_isNormalMapEnabled;
    bool lighting_isAOMapEnabled;
    bool lighting_isDisplacementMapEnabled;
};

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D aoTexture;
uniform vec2 textureScale;
uniform vec2 brushHighlightPos;
uniform float brushHighlightStrength;
uniform float brushHighlightRadius;
uniform float brushHighlightFalloff;

out vec4 FragColor;

vec3 calcBrushHighlight()
{
    float highlightRadius = brushHighlightRadius * 0.5f;
    float distFromHighlight = distance(texcoord.xz / textureScale, brushHighlightPos);
    
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

vec3 calcTriplanarBlend(vec3 normal)
{
    // bias towards Y-axis
    vec3 blend = vec3(pow(abs(normal.x), 6), pow(abs(normal.y), 1), pow(abs(normal.z), 6));
    blend = normalize(max(blend, 0.00001));
    blend /= blend.x + blend.y + blend.z;
    return blend;
}

void main()
{
    vec3 triplanarBlend = calcTriplanarBlend(vertexNormal);
    vec3 triplanarAxisSign = sign(vertexNormal);

    vec2 texcoord_x = vec2(texcoord.z * triplanarAxisSign.x, texcoord.y);
    vec2 texcoord_y = vec2(texcoord.x * triplanarAxisSign.y, texcoord.z);
    vec2 texcoord_z = vec2(texcoord.x * triplanarAxisSign.z, texcoord.y);

    // sample normal map
    vec3 texNormal = vec3(0);
    if (lighting_isNormalMapEnabled)
    {
        vec3 texNormal_x = (texture(normalTexture, texcoord_x).rgb * 2) - 1;
        vec3 texNormal_y = (texture(normalTexture, texcoord_y).rgb * 2) - 1;
        vec3 texNormal_z = (texture(normalTexture, texcoord_z).rgb * 2) - 1;

        // flip normals to correct for the flipped UVs
        texNormal_x.x *= triplanarAxisSign.x;
        texNormal_y.x *= triplanarAxisSign.y;
        texNormal_z.x *= triplanarAxisSign.z;

        texNormal = (texNormal_x * triplanarBlend.x) + (texNormal_y * triplanarBlend.y) + (texNormal_z * triplanarBlend.z);
    }
    vec3 normal = normalize(vertexNormal - (texNormal * 0.5f));

    // sample albedo texture
    vec3 albedo = vec3(1);
    if (lighting_isTextureEnabled)
    {
        vec3 albedo_x = texture(albedoTexture, texcoord_x).rgb;
        vec3 albedo_y = texture(albedoTexture, texcoord_y).rgb;
        vec3 albedo_z = texture(albedoTexture, texcoord_z).rgb;
        albedo = (albedo_x * triplanarBlend.x) + (albedo_y * triplanarBlend.y) + (albedo_z * triplanarBlend.z);
    }

    // calculate lighting
    float ambientLight = 0.1f;
    float nDotL = dot(normal, lighting_lightDir.xyz);
    float lightingAmplitude = lighting_isEnabled
        ? ambientLight + pow(0.5 + (nDotL * 0.5), 2)
        : 1.0f;

    // sample ambient occlusion texture
    float ao = 1;
    if (lighting_isAOMapEnabled)
    {
        float ao_x = texture(aoTexture, texcoord_x).r;
        float ao_y = texture(aoTexture, texcoord_y).r;
        float ao_z = texture(aoTexture, texcoord_z).r;
        ao = mix(0.6f, 1.0f,
            (ao_x * triplanarBlend.x) + (ao_y * triplanarBlend.y) + (ao_z * triplanarBlend.z));
    }

    // calculate final fragment color
    vec3 terrainColor = albedo * lightingAmplitude * ao;
    vec3 brushHighlight = calcBrushHighlight();
    FragColor = vec4(terrainColor + (brushHighlight * brushHighlightStrength), 1.0f);
}