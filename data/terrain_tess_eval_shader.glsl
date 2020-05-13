#version 410 core
layout(quads, fractional_even_spacing, cw) in;

layout(location = 0) in vec3 in_worldPos[];
layout(location = 1) in vec2 in_heightmapUV[];

patch in vec4 in_edgeMips;
patch in vec4 in_cornerMips;

uniform mat4 transform;
uniform sampler2D heightmapTexture;
uniform sampler2D displacementTexture;
uniform float terrainHeight;
uniform vec2 normalSampleOffset;
uniform vec2 textureScale;
uniform bool isDisplacementMapEnabled;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 texcoord;

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
    float baseHeight = textureCLod(heightmapTexture, uv, max(mip, 2.0f));
    float displacement =
        ((textureCLod(displacementTexture, uv * textureScale, mip * 3.0f) * 2.0f) - 1.0f)
        * (isDisplacementMapEnabled ? 0.0125f : 0.0f);
    return baseHeight + displacement;
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

    vec3 pos = lerp3D(in_worldPos[0], in_worldPos[1], in_worldPos[2], in_worldPos[3]);
    vec2 hUV = lerp2D(in_heightmapUV[0], in_heightmapUV[1], in_heightmapUV[2], in_heightmapUV[3]);
    pos.y = height(hUV, mip) * terrainHeight;
    gl_Position = transform * vec4(pos, 1.0f);

    float hL = height(vec2(hUV.x - normalSampleOffset.x, hUV.y), mip);
    float hR = height(vec2(hUV.x + normalSampleOffset.x, hUV.y), mip);
    float hD = height(vec2(hUV.x, hUV.y - normalSampleOffset.y), mip);
    float hU = height(vec2(hUV.x, hUV.y + normalSampleOffset.y), mip);
    normal = normalize(vec3(hR - hL, 0.1f, hD - hU));
    texcoord = hUV * textureScale;
}