#version 410 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;

uniform vec2 unitSize;

out vec3 worldPos;
out vec2 heightmapUV;

void main()
{
    worldPos = pos;
    heightmapUV = vec2((pos.x * unitSize.x) + 0.5f, (pos.z * unitSize.y) + 0.5f);
}