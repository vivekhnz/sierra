#version 430 core
layout(location = 0) in vec3 vertexNormal;
layout(location = 1) in vec3 texcoord;
layout(location = 2) in vec2 heights;

layout (std140, binding = 1) uniform Lighting
{
    vec4 lighting_lightDir;
    bool lighting_isEnabled;
    bool lighting_isTextureEnabled;
    bool lighting_isNormalMapEnabled;
    bool lighting_isAOMapEnabled;
    bool lighting_isDisplacementMapEnabled;
};

uniform int materialCount;
uniform vec3 terrainDimensions;

layout(binding = 1) uniform sampler2DArray albedoTextures;
layout(binding = 2) uniform sampler2DArray normalTextures;
layout(binding = 3) uniform sampler2DArray displacementTextures;
layout(binding = 4) uniform sampler2DArray aoTextures;

struct MaterialProperties
{
    vec2 textureSizeInWorldUnits;
    vec2 _padding;
    vec4 rampParams;
};
layout(std430, binding = 1) buffer materialPropsBuffer
{
    MaterialProperties materialProps[];
};

const int BRUSH_VIS_MODE_NONE = 0;
const int BRUSH_VIS_MODE_CURSOR_ONLY = 1;
const int BRUSH_VIS_MODE_SHOW_HEIGHT_DELTA = 2;
const int BRUSH_VIS_MODE_HIGHLIGHT_CURSOR = 3;

uniform int visualizationMode;
uniform vec2 cursorPos;
uniform float cursorRadius;
uniform float cursorFalloff;

out vec4 FragColor;

float calcRingOpacity(float radius, float width, float distFromCenter)
{
    return max(1 - abs((((distFromCenter - radius) / width) * 2) - 1), 0);
}

vec3 calcTriplanarBlend(vec3 normal)
{
    // bias towards Y-axis
    vec3 blend = vec3(pow(abs(normal.x), 6), pow(abs(normal.y), 1), pow(abs(normal.z), 6));
    blend = normalize(max(blend, 0.00001));
    blend /= blend.x + blend.y + blend.z;
    return blend;
}

float triplanar1D(float xVal, float yVal, float zVal, vec3 blend)
{
    return (xVal * blend.x) + (yVal * blend.y) + (zVal * blend.z);
}

vec3 triplanar3D(vec3 xVal, vec3 yVal, vec3 zVal, vec3 blend)
{
    return (xVal * blend.x) + (yVal * blend.y) + (zVal * blend.z);
}

void main()
{
    float actualHeight = heights.x;
    float previewHeight = heights.y;

    float altitude = texcoord.y / -terrainDimensions.y;
    float slope = 1 - vertexNormal.y;

    // calculate triplanar texture coordinates
    vec3 triBlend = calcTriplanarBlend(vertexNormal);
    vec3 triAxisSign = sign(vertexNormal);
    
    vec2 baseTexcoordsX = vec2(texcoord.z * triAxisSign.x, texcoord.y);
    vec2 baseTexcoordsY = vec2(texcoord.x * triAxisSign.y, texcoord.z);
    vec2 baseTexcoordsZ = vec2(texcoord.x * triAxisSign.z, texcoord.y);

    // blend materials based on slope, altitude and height
    vec3 material_albedo = vec3(0.72, 0.69, 0.65);
    vec3 material_normal = vec3(0);
    float material_ao = 1;

    float prevLayerDisplacement = 0.0f;
    for (int i = 0; i < materialCount; i++)
    {
        vec2 textureSizeInWorldUnits = materialProps[i].textureSizeInWorldUnits;
        vec3 materialTexcoordsX = vec3(baseTexcoordsX / textureSizeInWorldUnits.yy, i);
        vec3 materialTexcoordsY = vec3(baseTexcoordsY / textureSizeInWorldUnits.xy, i);
        vec3 materialTexcoordsZ = vec3(baseTexcoordsZ / textureSizeInWorldUnits.xy, i);

        float currentLayerDisplacement = triplanar1D(0,
            texture(displacementTextures, materialTexcoordsY).r * triAxisSign.y,
            0, triBlend);
        
        float blendAmount = 1;
        if (i > 0)
        {
            vec4 ramp = materialProps[i].rampParams;

            // blend based on slope and altitude
            float blend = clamp((slope - ramp.x) / (ramp.y - ramp.x), 0, 1);
            blend *= clamp((altitude - ramp.z) / (ramp.w - ramp.z), 0, 1);
            
            // blend based on material height
            float prevLayerHeight = prevLayerDisplacement * (1 - blend);
            float currentLayerHeight = currentLayerDisplacement * blend;
            blendAmount = currentLayerHeight / (prevLayerHeight + currentLayerHeight);
        }

        if (lighting_isTextureEnabled)
        {
            vec3 mat_albedo = triplanar3D(
                texture(albedoTextures, materialTexcoordsX).rgb,
                texture(albedoTextures, materialTexcoordsY).rgb,
                texture(albedoTextures, materialTexcoordsZ).rgb,
                triBlend);
            material_albedo = mix(material_albedo, mat_albedo, blendAmount);
        }
        if (lighting_isNormalMapEnabled)
        {
            vec3 mat_normal = triplanar3D(
                ((texture(normalTextures, materialTexcoordsX).rgb * 2) - 1) * vec3(triAxisSign.x, 1, 1),
                ((texture(normalTextures, materialTexcoordsY).rgb * 2) - 1) * vec3(triAxisSign.y, 1, 1),
                ((texture(normalTextures, materialTexcoordsZ).rgb * 2) - 1) * vec3(triAxisSign.z, 1, 1),
                triBlend);
            material_normal = mix(material_normal, mat_normal, blendAmount);
        }
        if (lighting_isAOMapEnabled)
        {
            float mat_ao = triplanar1D(
                texture(aoTextures, materialTexcoordsX).r,
                texture(aoTextures, materialTexcoordsY).r,
                texture(aoTextures, materialTexcoordsZ).r,
                triBlend);
            material_ao = mix(material_ao, mat_ao, blendAmount);
        }
        
        prevLayerDisplacement = currentLayerDisplacement;
    }

    // calculate lighting
    float ambientLight = 0.15f;
    vec3 normal = normalize(vertexNormal - (material_normal * 0.5f));
    float nDotL = dot(normal, lighting_lightDir.xyz);
    float lightingAmplitude = lighting_isEnabled
        ? ambientLight + pow(0.5 + (nDotL * 0.5), 2)
        : 1.0f;
    vec3 terrainColor = material_albedo * lightingAmplitude * mix(0.6, 1.0, material_ao);

    bool isCursorActive = true;
    bool isAdjustingParameter = false;
    bool isBrushActive = false;

    vec3 highlight = vec3(0);
    if (visualizationMode != BRUSH_VIS_MODE_NONE)
    {
        float outerRadius = cursorRadius * 0.5f;
        float innerRadius = outerRadius * cursorFalloff;
        vec2 normalizedUV = texcoord.xz / terrainDimensions.xz;
        float distFromCenter = distance(normalizedUV, cursorPos);

        float baseHighlightIntensity = 0;
        vec3 highlightColor = vec3(0.3);
        
        if (visualizationMode == BRUSH_VIS_MODE_SHOW_HEIGHT_DELTA)
        {
            float heightDelta = previewHeight - actualHeight;
            if (heightDelta > 0)
            {
                highlightColor = vec3(0, 0.5, 0.125);
            }
            else if (heightDelta < 0)
            {
                highlightColor = vec3(1, 0, 0.1f);
            }
            
            baseHighlightIntensity = min(abs(heightDelta) / 0.001f, 1) * 0.225;
        }
        else if (visualizationMode == BRUSH_VIS_MODE_HIGHLIGHT_CURSOR)
        {
            highlightColor = vec3(1, 0.509, 0.094);
            baseHighlightIntensity = (outerRadius - distFromCenter) / (outerRadius - innerRadius);
            baseHighlightIntensity = clamp(baseHighlightIntensity, 0, 1);
            baseHighlightIntensity *= 0.2;
        }
        highlight = highlightColor * baseHighlightIntensity;

        // outline thickness is based on depth
        float outlineWidth = max(min(0.003 - (0.07 * gl_FragCoord.w), 0.002), 0.0005);
        
        float outlineIntensity = 0;
        outlineIntensity += calcRingOpacity(outerRadius, outlineWidth, distFromCenter);
        outlineIntensity += calcRingOpacity(innerRadius, outlineWidth, distFromCenter);
        
        float outlineShadowIntensity = 0;
        outlineShadowIntensity += calcRingOpacity(
            outerRadius - (outlineWidth * 1.5), outlineWidth * 4, distFromCenter);
        outlineShadowIntensity += calcRingOpacity(
            innerRadius - (outlineWidth * 1.5), outlineWidth * 4, distFromCenter);

        highlight += vec3(-0.05 * outlineShadowIntensity);
        highlight += mix(highlightColor, vec3(1), 0.3) * outlineIntensity;
    }

    FragColor = vec4(terrainColor + highlight, 1);
}