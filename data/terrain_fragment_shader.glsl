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

uniform vec3 terrainDimensions;
uniform vec2 brushHighlightPos;
uniform float brushHighlightStrength;
uniform float brushHighlightRadius;
uniform float brushHighlightFalloff;

layout(binding = 1) uniform sampler2DArray albedoTextures;
layout(binding = 2) uniform sampler2DArray normalTextures;
layout(binding = 3) uniform sampler2D mat1_displacement;
layout(binding = 4) uniform sampler2DArray aoTextures;
uniform vec2 mat1_textureSizeInWorldUnits;

layout(binding = 6) uniform sampler2D mat2_normal;
layout(binding = 7) uniform sampler2D mat2_displacement;
uniform vec2 mat2_textureSizeInWorldUnits;
uniform vec4 mat2_rampParams;

layout(binding = 10) uniform sampler2D mat3_normal;
layout(binding = 11) uniform sampler2D mat3_displacement;
uniform vec2 mat3_textureSizeInWorldUnits;
uniform vec4 mat3_rampParams;

out vec4 FragColor;

struct TriplanarTextureCoordinates
{
    vec2 x;
    vec2 y;
    vec2 z;
};
struct MaterialData
{
    vec3 albedo;
    vec3 normal;
    float ao;
};

vec3 calcBrushHighlight()
{
    float highlightRadius = brushHighlightRadius * 0.5f;
    vec2 normalizedUV = texcoord.xz / terrainDimensions.xz;
    float distFromHighlight = distance(normalizedUV, brushHighlightPos);
    
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

float triplanar1D(float xVal, float yVal, float zVal, vec3 blend)
{
    return (xVal * blend.x) + (yVal * blend.y) + (zVal * blend.z);
}

vec3 triplanar3D(vec3 xVal, vec3 yVal, vec3 zVal, vec3 blend)
{
    return (xVal * blend.x) + (yVal * blend.y) + (zVal * blend.z);
}

float calcMaterialBlend(float slope, float altitude, vec4 ramp, float lowerMatDisplacement, float upperMatDisplacement)
{
    // blend based on slope and altitude
    float blend = clamp((slope - ramp.x) / (ramp.y - ramp.x), 0, 1);
    blend *= clamp((altitude - ramp.z) / (ramp.w - ramp.z), 0, 1);
    
    // blend based on material height
    float lower_mat_height = lowerMatDisplacement * (1 - blend);
    float upper_mat_height = upperMatDisplacement * blend;
    return upper_mat_height / (lower_mat_height + upper_mat_height);
}

MaterialData blendMaterial(MaterialData material, float matBlendAmt,
    vec2 texcoord_x, vec2 texcoord_y, vec2 texcoord_z, vec3 triBlend, vec3 triAxisSign,
    int layerIndex)
{
    vec3 texcoord_x_layered = vec3(texcoord_x, layerIndex);
    vec3 texcoord_y_layered = vec3(texcoord_y, layerIndex);
    vec3 texcoord_z_layered = vec3(texcoord_z, layerIndex);

    if (lighting_isTextureEnabled)
    {
        vec3 mat_albedo = triplanar3D(
            texture(albedoTextures, texcoord_x_layered).rgb,
            texture(albedoTextures, texcoord_y_layered).rgb,
            texture(albedoTextures, texcoord_z_layered).rgb,
            triBlend);
        material.albedo = mix(material.albedo, mat_albedo, matBlendAmt);
    }
    if (lighting_isNormalMapEnabled)
    {
        vec3 mat_normal = triplanar3D(
            ((texture(normalTextures, texcoord_x_layered).rgb * 2) - 1) * vec3(triAxisSign.x, 1, 1),
            ((texture(normalTextures, texcoord_y_layered).rgb * 2) - 1) * vec3(triAxisSign.y, 1, 1),
            ((texture(normalTextures, texcoord_z_layered).rgb * 2) - 1) * vec3(triAxisSign.z, 1, 1),
            triBlend);
        material.normal = mix(material.normal, mat_normal, matBlendAmt);
    }
    if (lighting_isAOMapEnabled)
    {
        float mat_ao = triplanar1D(
            texture(aoTextures, texcoord_x_layered).r,
            texture(aoTextures, texcoord_y_layered).r,
            texture(aoTextures, texcoord_z_layered).r,
            triBlend);
        material.ao = mix(material.ao, mat_ao, matBlendAmt);
    }
    return material;
}

void main()
{
    float altitude = texcoord.y / -terrainDimensions.y;
    float slope = 1 - vertexNormal.y;
    
    // setup material arrays
    vec2 materialTextureSizes[3];
    materialTextureSizes[0] = mat1_textureSizeInWorldUnits;
    materialTextureSizes[1] = mat2_textureSizeInWorldUnits;
    materialTextureSizes[2] = mat3_textureSizeInWorldUnits;

    vec4 materialRampParams[2];
    materialRampParams[0] = mat2_rampParams;
    materialRampParams[1] = mat3_rampParams;

    // calculate triplanar texture coordinates
    vec3 triBlend = calcTriplanarBlend(vertexNormal);
    vec3 triAxisSign = sign(vertexNormal);
    
    TriplanarTextureCoordinates baseTexcoords;
    baseTexcoords.x = vec2(texcoord.z * triAxisSign.x, texcoord.y);
    baseTexcoords.y = vec2(texcoord.x * triAxisSign.y, texcoord.z);
    baseTexcoords.z = vec2(texcoord.x * triAxisSign.z, texcoord.y);

    TriplanarTextureCoordinates materialTexcoords[3];
    for (int i = 0; i < 3; i++)
    {
        materialTexcoords[i].x = baseTexcoords.x / materialTextureSizes[i].yy;
        materialTexcoords[i].y = baseTexcoords.y / materialTextureSizes[i].xy;
        materialTexcoords[i].z = baseTexcoords.z / materialTextureSizes[i].xy;
    }

    // calculate displacement of each material
    float materialDisplacements[3];
    materialDisplacements[0] = triplanar1D(
        0, texture(mat1_displacement, materialTexcoords[0].y).r * triAxisSign.y, 0,
        triBlend);
    materialDisplacements[1] = triplanar1D(
        0, texture(mat2_displacement, materialTexcoords[1].y).r * triAxisSign.y, 0,
        triBlend);
    materialDisplacements[2] = triplanar1D(
        0, texture(mat3_displacement, materialTexcoords[2].y).r * triAxisSign.y, 0,
        triBlend);

    // blend materials based on slope, altitude and height
    MaterialData material;
    material.albedo = vec3(1);
    material.normal = vec3(0);
    material.ao = 1;

    material = blendMaterial(material, 1,
        materialTexcoords[0].x, materialTexcoords[0].y, materialTexcoords[0].z, triBlend, triAxisSign, 0);
        
    float mat2_blend = calcMaterialBlend(slope, altitude, materialRampParams[0], materialDisplacements[0], materialDisplacements[1]);
    material = blendMaterial(material, mat2_blend,
        materialTexcoords[1].x, materialTexcoords[1].y, materialTexcoords[1].z, triBlend, triAxisSign, 1);
        
    float mat3_blend = calcMaterialBlend(slope, altitude, materialRampParams[1], materialDisplacements[1], materialDisplacements[2]);
    material = blendMaterial(material, mat3_blend,
        materialTexcoords[2].x, materialTexcoords[2].y, materialTexcoords[2].z, triBlend, triAxisSign, 2);

    // calculate lighting
    float ambientLight = 0.1f;
    vec3 normal = normalize(vertexNormal - (material.normal * 0.5f));
    float nDotL = dot(normal, lighting_lightDir.xyz);
    float lightingAmplitude = lighting_isEnabled
        ? ambientLight + pow(0.5 + (nDotL * 0.5), 2)
        : 1.0f;

    // calculate final fragment color
    vec3 terrainColor = material.albedo * lightingAmplitude * mix(0.6, 1.0, material.ao);
    vec3 brushHighlight = calcBrushHighlight();
    FragColor = vec4(terrainColor + (brushHighlight * brushHighlightStrength), 1.0f);
}