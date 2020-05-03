#version 410 core
layout(quads, fractional_even_spacing, cw) in;

layout(location = 0) in vec3 in_worldPos[];
layout(location = 1) in vec2 in_heightmapUV[];

uniform mat4 transform;
uniform sampler2D heightmapTexture;
uniform float terrainHeight;
uniform vec2 normalSampleOffset;
uniform vec2 textureScale;

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

void main()
{
    vec3 pos = lerp3D(in_worldPos[0], in_worldPos[1], in_worldPos[2], in_worldPos[3]);
    vec2 hUV = lerp2D(in_heightmapUV[0], in_heightmapUV[1], in_heightmapUV[2], in_heightmapUV[3]);
    pos.y = texture(heightmapTexture, hUV).x * terrainHeight;
    gl_Position = transform * vec4(pos, 1.0f);

    float hL = texture(heightmapTexture, vec2(hUV.x - normalSampleOffset.x, hUV.y)).x;
    float hR = texture(heightmapTexture, vec2(hUV.x + normalSampleOffset.x, hUV.y)).x;
    float hD = texture(heightmapTexture, vec2(hUV.x, hUV.y - normalSampleOffset.y)).x;
    float hU = texture(heightmapTexture, vec2(hUV.x, hUV.y + normalSampleOffset.y)).x;
    normal = normalize(vec3(hR - hL, 0.1f, hD - hU));
    texcoord = hUV * textureScale;
}