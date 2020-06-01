#version 430 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

layout(location = 0) out int id;
layout(location = 1) out vec3 worldPos;
layout(location = 2) out vec2 heightmapUV;

void main()
{
    id = gl_VertexID;
    worldPos = pos;
    heightmapUV = uv;
}