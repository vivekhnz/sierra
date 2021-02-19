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

uniform vec3 terrainDimensions;

layout(binding = 0) uniform sampler2D heightmapTexture;
layout(binding = 3) uniform sampler2DArray displacementTextures;
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
float getDisplacement(vec2 uv, int layerIdx, float mip)
{
    vec3 uvLayered = vec3(uv, layerIdx);

    return mix(
        textureLod(displacementTextures, uvLayered, floor(mip)).x,
        textureLod(displacementTextures, uvLayered, ceil(mip)).x,
        fract(mip));
}
float height(vec2 uv)
{
    return textureLod(heightmapTexture, uv, 2.0f).x;
}

void main()
{
    vec2 hUV = lerp2D(in_heightmapUV[0], in_heightmapUV[1], in_heightmapUV[2], in_heightmapUV[3]);
    vec2 textureSizeInWorldUnits = materialProps[0].textureSizeInWorldUnits;
    vec2 texcoord = hUV.xy * terrainDimensions.xz / textureSizeInWorldUnits.xy;

    vec2 normalSampleOffset = 1 / terrainDimensions.xz;
    float hL = height(vec2(hUV.x - normalSampleOffset.x, hUV.y));
    float hR = height(vec2(hUV.x + normalSampleOffset.x, hUV.y));
    float hD = height(vec2(hUV.x, hUV.y - normalSampleOffset.y));
    float hU = height(vec2(hUV.x, hUV.y + normalSampleOffset.y));
    vec3 normal = normalize(vec3(hR - hL, normalSampleOffset.x * 2, hD - hU));
    
    vec3 pos = lerp3D(in_worldPos[0], in_worldPos[1], in_worldPos[2], in_worldPos[3]);
    pos.y = height(hUV) * terrainDimensions.y;
    if (lighting_isDisplacementMapEnabled)
    {
        float scaledMip = log2(terrainDimensions.x / textureSizeInWorldUnits.x);
        float displacement =
            ((getDisplacement(texcoord, 0, scaledMip) * 2.0f) - 1.0f);
        pos += normal * displacement * 0.1f;
    }
    
    gl_Position = camera_transform * vec4(pos, 1.0f);
}