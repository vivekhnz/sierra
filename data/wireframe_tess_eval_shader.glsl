#version 410 core
layout(triangles, fractional_even_spacing, ccw) in;

layout(location = 0) in vec3 in_worldPos[];
layout(location = 1) in vec2 in_heightmapUV[];

uniform mat4 transform;
uniform sampler2D heightmapTexture;
uniform float terrainHeight;

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
    vec3 pos = lerp3D(in_worldPos[0], in_worldPos[1], in_worldPos[2]);
    vec2 hUV = lerp2D(in_heightmapUV[0], in_heightmapUV[1], in_heightmapUV[2]);
    pos.y = texture(heightmapTexture, hUV).x * terrainHeight;
    gl_Position = transform * vec4(pos, 1.0f);
}