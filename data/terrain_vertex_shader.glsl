#version 410 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec3 worldPos;
layout(location = 1) out vec2 heightmapUV;

void main()
{
    worldPos = pos;
    heightmapUV = uv;
}