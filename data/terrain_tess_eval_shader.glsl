#version 410 core
layout(triangles, equal_spacing, ccw) in;

in vec3 worldPos[];
in vec2 heightmapUV[];

uniform mat4 transform;
uniform sampler2D heightmapTexture;
uniform float terrainHeight;
uniform vec2 normalSampleOffset;
uniform vec2 textureScale;

out vec3 normal;
out vec2 texcoord;

vec3 lerp3D(vec3 a, vec3 b, vec3 c)
{
    return (a * gl_TessCoord.x) + (b * gl_TessCoord.y) + (c * gl_TessCoord.z);
}
vec2 lerp2D(vec2 a, vec2 b, vec2 c)
{
    return (a * gl_TessCoord.x) + (b * gl_TessCoord.y) + (c * gl_TessCoord.z);
}

void main()
{
    vec3 pos = lerp3D(worldPos[0], worldPos[1], worldPos[2]);
    vec2 hUV = lerp2D(heightmapUV[0], heightmapUV[1], heightmapUV[2]);
    pos.y = texture(heightmapTexture, hUV).x * terrainHeight;
    gl_Position = transform * vec4(pos, 1.0f);

    float hL = texture(heightmapTexture, vec2(hUV.x - normalSampleOffset.x, hUV.y)).x;
    float hR = texture(heightmapTexture, vec2(hUV.x + normalSampleOffset.x, hUV.y)).x;
    float hD = texture(heightmapTexture, vec2(hUV.x, hUV.y - normalSampleOffset.y)).x;
    float hU = texture(heightmapTexture, vec2(hUV.x, hUV.y + normalSampleOffset.y)).x;
    normal = normalize(vec3(hR - hL, normalSampleOffset.x, hD - hU));
    texcoord = hUV * textureScale;
}