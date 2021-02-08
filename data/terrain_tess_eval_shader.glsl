#version 430 core
layout(quads, fractional_even_spacing, cw) in;

layout(location = 0) in vec3 in_worldPos[];
layout(location = 1) in vec2 in_heightmapUV[];

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};
layout (std140, binding = 1) uniform Lighting
{
    vec4 lighting_lightDir;
    bool lighting_isEnabled;
    bool lighting_isTextureEnabled;
    bool lighting_isNormalMapEnabled;
    bool lighting_isAOMapEnabled;
    bool lighting_isDisplacementMapEnabled;
};

layout(binding = 0) uniform sampler2D heightmapTexture;
uniform vec3 terrainDimensions;

layout(binding = 3) uniform sampler2D mat1_displacement;
uniform vec2 mat1_textureSizeInWorldUnits;

layout(binding = 7) uniform sampler2D mat2_displacement;
uniform vec2 mat2_textureSizeInWorldUnits;
uniform vec4 mat2_rampParams;

layout(binding = 11) uniform sampler2D mat3_displacement;
uniform vec2 mat3_textureSizeInWorldUnits;
uniform vec4 mat3_rampParams;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 texcoord;

struct TriplanarTextureCoordinates
{
    vec2 x;
    vec2 y;
    vec2 z;
};

vec3 lerp3D(vec3 a, vec3 b, vec3 c, vec3 d)
{
    return mix(mix(a, d, gl_TessCoord.x), mix(b, c, gl_TessCoord.x), gl_TessCoord.y);
}
vec2 lerp2D(vec2 a, vec2 b, vec2 c, vec2 d)
{
    return mix(mix(a, d, gl_TessCoord.x), mix(b, c, gl_TessCoord.x), gl_TessCoord.y);
}
float lerp1D(float a, float b, float c, float d)
{
    return mix(mix(a, d, gl_TessCoord.x), mix(b, c, gl_TessCoord.x), gl_TessCoord.y);
}
float textureCLod(sampler2D texture, vec2 uv, float mip)
{
    return mix(
        textureLod(texture, uv, floor(mip)).x,
        textureLod(texture, uv, ceil(mip)).x,
        fract(mip));
}
float height(vec2 uv)
{
    return textureCLod(heightmapTexture, uv, 2.0f);
}
vec3 calcTriplanarBlend(vec3 normal)
{
    // bias towards Y-axis
    vec3 blend = vec3(pow(abs(normal.x), 6), pow(abs(normal.y), 1), pow(abs(normal.z), 6));
    blend = normalize(max(blend, 0.00001));
    blend /= blend.x + blend.y + blend.z;
    return blend;
}
vec3 triplanar3D(vec3 xVal, vec3 yVal, vec3 zVal, vec3 blend)
{
    return (xVal * blend.x) + (yVal * blend.y) + (zVal * blend.z);
}

void main()
{
    // calculate normal
    vec2 hUV = lerp2D(in_heightmapUV[0], in_heightmapUV[1], in_heightmapUV[2], in_heightmapUV[3]);
    float altitude = height(hUV);
    vec2 normalSampleOffset = 1 / terrainDimensions.xz;
    float hL = height(vec2(hUV.x - normalSampleOffset.x, hUV.y));
    float hR = height(vec2(hUV.x + normalSampleOffset.x, hUV.y));
    float hD = height(vec2(hUV.x, hUV.y - normalSampleOffset.y));
    float hU = height(vec2(hUV.x, hUV.y + normalSampleOffset.y));
    normal = normalize(vec3(hR - hL, normalSampleOffset.x * 2, hD - hU));
    float slope = 1 - normal.y;
    
    // setup material arrays
    vec2 materialTextureSizes[3];
    materialTextureSizes[0] = mat1_textureSizeInWorldUnits;
    materialTextureSizes[1] = mat2_textureSizeInWorldUnits;
    materialTextureSizes[2] = mat3_textureSizeInWorldUnits;

    vec4 materialRampParams[2];
    materialRampParams[0] = mat2_rampParams;
    materialRampParams[1] = mat3_rampParams;

    float materialScaledMips[3];
    for (int i = 0; i < 3; i++)
    {
        materialScaledMips[i] = log2(terrainDimensions.x / materialTextureSizes[i].x);
    }
    
    // calculate texture coordinates
    vec3 triBlend = calcTriplanarBlend(normal);
    vec3 triAxisSign = sign(normal);
    texcoord = vec3(
        hUV.x * terrainDimensions.x,
        -altitude * terrainDimensions.y,
        hUV.y * terrainDimensions.z);

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
    
    vec3 pos = lerp3D(in_worldPos[0], in_worldPos[1], in_worldPos[2], in_worldPos[3]);
    pos.y = altitude * terrainDimensions.y;

    if (lighting_isDisplacementMapEnabled)
    {
        vec3 displacement_mat1 = triplanar3D(
            vec3(textureCLod(mat1_displacement, materialTexcoords[0].x, materialScaledMips[0]) * -triAxisSign.x, 0, 0),
            vec3(0, textureCLod(mat1_displacement, materialTexcoords[0].y, materialScaledMips[0]) * triAxisSign.y, 0),
            vec3(0, 0, textureCLod(mat1_displacement, materialTexcoords[0].z, materialScaledMips[0]) * triAxisSign.z),
            triBlend);
        vec3 displacement = displacement_mat1;

        vec3 displacement_mat2 = triplanar3D(
            vec3(textureCLod(mat2_displacement, materialTexcoords[1].x, materialScaledMips[1]) * -triAxisSign.x, 0, 0),
            vec3(0, textureCLod(mat2_displacement, materialTexcoords[1].y, materialScaledMips[1]) * triAxisSign.y, 0),
            vec3(0, 0, textureCLod(mat2_displacement, materialTexcoords[1].z, materialScaledMips[1]) * triAxisSign.z),
            triBlend);
        float mat2_blend = clamp((slope - materialRampParams[0].x) / (materialRampParams[0].y - materialRampParams[0].x), 0, 1);
        mat2_blend *= clamp((altitude - materialRampParams[0].z) / (materialRampParams[0].w - materialRampParams[0].z), 0, 1);
        displacement = mix(displacement, displacement_mat2, mat2_blend);

        vec3 displacement_mat3 = triplanar3D(
            vec3(textureCLod(mat3_displacement, materialTexcoords[2].x, materialScaledMips[2]) * -triAxisSign.x, 0, 0),
            vec3(0, textureCLod(mat3_displacement, materialTexcoords[2].y, materialScaledMips[2]) * triAxisSign.y, 0),
            vec3(0, 0, textureCLod(mat3_displacement, materialTexcoords[2].z, materialScaledMips[2]) * triAxisSign.z),
            triBlend);
        float mat3_blend = clamp((slope - materialRampParams[1].x) / (materialRampParams[1].y - materialRampParams[1].x), 0, 1);
        mat3_blend *= clamp((altitude - materialRampParams[1].z) / (materialRampParams[1].w - materialRampParams[1].z), 0, 1);
        displacement = mix(displacement, displacement_mat3, mat3_blend);

        pos += displacement * 0.8;
    }
    
    gl_Position = camera_transform * vec4(pos, 1.0f);
}