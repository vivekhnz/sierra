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

uniform sampler2D heightmapTexture;
uniform sampler2D mat1_displacement;
uniform sampler2D mat2_displacement;
uniform vec3 terrainDimensions;
uniform vec2 textureSizeInWorldUnits;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 texcoord;

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
    // calculate mip level
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

    // calculate normal
    vec2 hUV = lerp2D(in_heightmapUV[0], in_heightmapUV[1], in_heightmapUV[2], in_heightmapUV[3]);
    vec2 normalSampleOffset = 1 / terrainDimensions.xz;
    float hL = height(vec2(hUV.x - normalSampleOffset.x, hUV.y), mip);
    float hR = height(vec2(hUV.x + normalSampleOffset.x, hUV.y), mip);
    float hD = height(vec2(hUV.x, hUV.y - normalSampleOffset.y), mip);
    float hU = height(vec2(hUV.x, hUV.y + normalSampleOffset.y), mip);
    normal = normalize(vec3(hR - hL, normalSampleOffset.x * 2, hD - hU));
    float slope = normal.y;

    // calculate texture coordinates
    float heightNormalised = height(hUV, mip);
    vec3 triBlend = calcTriplanarBlend(normal);
    vec3 triAxisSign = sign(normal);
    texcoord = vec3(
        hUV.x * terrainDimensions.x / textureSizeInWorldUnits.x,
        -heightNormalised * terrainDimensions.y / textureSizeInWorldUnits.y,
        hUV.y * terrainDimensions.z / textureSizeInWorldUnits.y);
    vec2 texcoord_x = vec2(texcoord.z * triAxisSign.x, texcoord.y);
    vec2 texcoord_y = vec2(texcoord.x * triAxisSign.y, texcoord.z);
    vec2 texcoord_z = vec2(texcoord.x * triAxisSign.z, texcoord.y);
    
    vec3 pos = lerp3D(in_worldPos[0], in_worldPos[1], in_worldPos[2], in_worldPos[3]);
    pos.y = heightNormalised * terrainDimensions.y;
    if (lighting_isDisplacementMapEnabled)
    {
        float scaledMip = mip + log2(terrainDimensions.x / textureSizeInWorldUnits.x);

        vec3 displacement_mat1 = triplanar3D(
            vec3(textureCLod(mat1_displacement, texcoord_x, scaledMip) * -triAxisSign.x, 0, 0),
            vec3(0, textureCLod(mat1_displacement, texcoord_y, scaledMip) * triAxisSign.y, 0),
            vec3(0, 0, textureCLod(mat1_displacement, texcoord_z, scaledMip) * triAxisSign.z),
            triBlend);
        vec3 displacement_mat2 = triplanar3D(
            vec3(textureCLod(mat2_displacement, texcoord_x / 3, scaledMip) * -triAxisSign.x, 0, 0),
            vec3(0, textureCLod(mat2_displacement, texcoord_y / 3, scaledMip) * triAxisSign.y, 0),
            vec3(0, 0, textureCLod(mat2_displacement, texcoord_z / 3, scaledMip) * triAxisSign.z),
            triBlend);

        vec3 displacement = mix(displacement_mat2, displacement_mat1, slope);
        pos += ((displacement * 2) - 1) * 0.1;
    }
    
    gl_Position = camera_transform * vec4(pos, 1.0f);
}