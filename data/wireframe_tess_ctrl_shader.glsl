#version 410 core
layout(vertices = 3) out;

layout(location = 0) in vec3 in_worldPos[];
layout(location = 1) in vec2 in_heightmapUV[];

uniform float tessellationLevel;

layout(location = 0) out vec3 out_worldPos[];
layout(location = 1) out vec2 out_heightmapUV[];

void main()
{
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = tessellationLevel;
        gl_TessLevelOuter[1] = tessellationLevel;
        gl_TessLevelOuter[2] = tessellationLevel;
        gl_TessLevelInner[0] = tessellationLevel;
    }
    out_worldPos[gl_InvocationID] = in_worldPos[gl_InvocationID];
    out_heightmapUV[gl_InvocationID] = in_heightmapUV[gl_InvocationID];
}