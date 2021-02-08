#version 430 core
layout(vertices = 4) out;

layout(location = 0) in int in_id[];
layout(location = 1) in vec3 in_worldPos[];
layout(location = 2) in vec2 in_heightmapUV[];

layout(std430, binding = 0) buffer tessellationLevelBuffer
{
    vec4 vertEdgeTessLevels[];
};

layout(location = 0) out vec3 out_worldPos[];
layout(location = 1) out vec2 out_heightmapUV[];

void main()
{
    out_worldPos[gl_InvocationID] = in_worldPos[gl_InvocationID];
    out_heightmapUV[gl_InvocationID] = in_heightmapUV[gl_InvocationID];
    
    if (gl_InvocationID == 0)
    {
        vec4 avertEdgeTessLevels = vertEdgeTessLevels[in_id[0]];
        vec4 bvertEdgeTessLevels = vertEdgeTessLevels[in_id[1]];
        vec4 cvertEdgeTessLevels = vertEdgeTessLevels[in_id[2]];
        vec4 dvertEdgeTessLevels = vertEdgeTessLevels[in_id[3]];
        
        if (avertEdgeTessLevels.x < 0 && avertEdgeTessLevels.y < 0 &&
            cvertEdgeTessLevels.z < 0 && cvertEdgeTessLevels.w < 0)
        {
            // cull the patch
            gl_TessLevelOuter[0] = 0;
            gl_TessLevelOuter[1] = 0;
            gl_TessLevelOuter[2] = 0;
            gl_TessLevelOuter[3] = 0;
            gl_TessLevelInner[0] = 0;
            gl_TessLevelInner[1] = 0;
        }
        else
        {
            // at least one edge is not cullable
            // need to use the absolute value of each tessellation level as they could be
            // negative if the edge was marked cullable
            gl_TessLevelOuter[0] = abs(avertEdgeTessLevels.y); // AB
            gl_TessLevelOuter[1] = abs(avertEdgeTessLevels.x); // AD
            gl_TessLevelOuter[2] = abs(cvertEdgeTessLevels.w); // CD
            gl_TessLevelOuter[3] = abs(cvertEdgeTessLevels.z); // BC
            gl_TessLevelInner[0] = max(abs(avertEdgeTessLevels.x), abs(cvertEdgeTessLevels.z));
            gl_TessLevelInner[1] = max(abs(avertEdgeTessLevels.y), abs(cvertEdgeTessLevels.w));
        }
    }
}