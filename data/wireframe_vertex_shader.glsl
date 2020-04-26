#version 410 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;

out vec3 worldPos;
out vec2 heightmapUV;

void main()
{
    worldPos = pos;
    heightmapUV = uv;
}