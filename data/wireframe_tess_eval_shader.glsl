#version 430 core
layout(quads, fractional_even_spacing, cw) in;

layout(location = 0) in vec3 in_worldPos[];
layout(location = 1) in vec2 in_heightmapUV[];

patch in vec4 in_edgeMips;
patch in vec4 in_cornerMips;

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
layout(binding = 3) uniform sampler2D mat1_displacement;
uniform vec3 terrainDimensions;
uniform vec2 mat1_textureSizeInWorldUnits;

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
float height(vec2 uv, float mip)
{
    return textureCLod(heightmapTexture, uv, max(mip, 2.0f));
}

void main()
{
    float centerMip = lerp1D(in_edgeMips.x, in_edgeMips.y, in_edgeMips.z, in_edgeMips.w);
    float mipValues[9] = float[9]
    (
        in_cornerMips.x, in_edgeMips.y, in_cornerMips.w,
        in_edgeMips.x, centerMip, in_edgeMips.z,
        in_cornerMips.y, in_edgeMips.w, in_cornerMips.z
    );
    float mipIndex = int(
        floor(gl_TessCoord.x) + ceil(gl_TessCoord.x)) +
        ((floor(gl_TessCoord.y) + ceil(gl_TessCoord.y)) * 3
    );
    float mip = mipValues[int(mipIndex)];

    vec2 hUV = lerp2D(in_heightmapUV[0], in_heightmapUV[1], in_heightmapUV[2], in_heightmapUV[3]);
    vec2 texcoord = hUV.xy * terrainDimensions.xz / mat1_textureSizeInWorldUnits.xy;

    vec2 normalSampleOffset = 1 / terrainDimensions.xz;
    float hL = height(vec2(hUV.x - normalSampleOffset.x, hUV.y), mip);
    float hR = height(vec2(hUV.x + normalSampleOffset.x, hUV.y), mip);
    float hD = height(vec2(hUV.x, hUV.y - normalSampleOffset.y), mip);
    float hU = height(vec2(hUV.x, hUV.y + normalSampleOffset.y), mip);
    vec3 normal = normalize(vec3(hR - hL, normalSampleOffset.x * 2, hD - hU));
    
    vec3 pos = lerp3D(in_worldPos[0], in_worldPos[1], in_worldPos[2], in_worldPos[3]);
    pos.y = height(hUV, mip) * terrainDimensions.y;
    if (lighting_isDisplacementMapEnabled)
    {
        float scaledMip = mip + log2(terrainDimensions.x / mat1_textureSizeInWorldUnits.x);
        float displacement =
            ((textureCLod(mat1_displacement, texcoord, scaledMip) * 2.0f) - 1.0f);
        pos += normal * displacement * 0.1f;
    }
    
    gl_Position = camera_transform * vec4(pos, 1.0f);
}