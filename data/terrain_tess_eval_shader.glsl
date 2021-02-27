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

uniform int materialCount;
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
    
    // calculate texture coordinates
    vec3 triBlend = calcTriplanarBlend(normal);
    vec3 triAxisSign = sign(normal);
    texcoord = vec3(
        hUV.x * terrainDimensions.x,
        -altitude * terrainDimensions.y,
        hUV.y * terrainDimensions.z);

    vec2 baseTexcoordsX = vec2(texcoord.z * triAxisSign.x, texcoord.y);
    vec2 baseTexcoordsY = vec2(texcoord.x * triAxisSign.y, texcoord.z);
    vec2 baseTexcoordsZ = vec2(texcoord.x * triAxisSign.z, texcoord.y);
    
    vec3 pos = lerp3D(in_worldPos[0], in_worldPos[1], in_worldPos[2], in_worldPos[3]);
    pos.y = altitude * terrainDimensions.y;

    if (lighting_isDisplacementMapEnabled)
    {
        vec3 displacement = vec3(0);
        for (int i = 0; i < materialCount; i++)
        {
            vec2 textureSizeInWorldUnits = materialProps[i].textureSizeInWorldUnits;

            vec2 materialTexcoordsX = baseTexcoordsX / textureSizeInWorldUnits.yy;
            vec2 materialTexcoordsY = baseTexcoordsY / textureSizeInWorldUnits.xy;
            vec2 materialTexcoordsZ = baseTexcoordsZ / textureSizeInWorldUnits.xy;

            float scaledMip = log2(terrainDimensions.x / textureSizeInWorldUnits.x);
            vec3 currentLayerDisplacement = triplanar3D(
                vec3(getDisplacement(materialTexcoordsX, i, scaledMip) * -triAxisSign.x, 0, 0),
                vec3(0, getDisplacement(materialTexcoordsY, i, scaledMip) * triAxisSign.y, 0),
                vec3(0, 0, getDisplacement(materialTexcoordsZ, i, scaledMip) * triAxisSign.z),
                triBlend);
            
            if (i == 0)
            {
                displacement = currentLayerDisplacement;
            }
            else
            {
                vec4 ramp = materialProps[i].rampParams;
                float blendAmount = clamp((slope - ramp.x) / (ramp.y - ramp.x), 0, 1);
                blendAmount *= clamp((altitude - ramp.z) / (ramp.w - ramp.z), 0, 1);
                displacement = mix(displacement, currentLayerDisplacement, blendAmount);
            }
        }

        pos += displacement * 0.8;
    }
    
    gl_Position = camera_transform * vec4(pos, 1.0f);
}