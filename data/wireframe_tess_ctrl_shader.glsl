#version 430 core
layout(vertices = 4) out;

layout(location = 0) in int in_id[];
layout(location = 1) in vec3 in_worldPos[];
layout(location = 2) in vec2 in_heightmapUV[];

uniform vec2 heightmapSize;

struct VertexEdgeData
{
    vec4 tessLevels;
    vec4 uvLengths;
};
layout(std430, binding = 0) buffer tessellationLevelBuffer
{
    VertexEdgeData vertEdgeData[];
};

layout(location = 0) out vec3 out_worldPos[];
layout(location = 1) out vec2 out_heightmapUV[];

patch out vec4 out_edgeMips;
patch out vec4 out_cornerMips;

float calcCornerMips(VertexEdgeData vertEdge)
{
    float T = max(
        max(vertEdge.tessLevels.x, vertEdge.tessLevels.y),
        max(vertEdge.tessLevels.z, vertEdge.tessLevels.w));
    vec4 scaledLengths = step(vertEdge.tessLevels / T, vec4(1.0f)) * vertEdge.uvLengths;
    float L = max(
        max(scaledLengths.x, scaledLengths.y),
        max(scaledLengths.z, scaledLengths.w));
    return log2(L * length(heightmapSize) / T) + 1;
}

void main()
{
    out_worldPos[gl_InvocationID] = in_worldPos[gl_InvocationID];
    out_heightmapUV[gl_InvocationID] = in_heightmapUV[gl_InvocationID];
    
    if (gl_InvocationID == 0)
    {
        VertexEdgeData aEdges = vertEdgeData[in_id[0]];
        VertexEdgeData bEdges = vertEdgeData[in_id[1]];
        VertexEdgeData cEdges = vertEdgeData[in_id[2]];
        VertexEdgeData dEdges = vertEdgeData[in_id[3]];
        float heightmapLength = length(heightmapSize);
        
        if (aEdges.tessLevels.x < 0 && aEdges.tessLevels.y < 0 &&
            cEdges.tessLevels.z < 0 && cEdges.tessLevels.w < 0)
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
            gl_TessLevelOuter[0] = abs(aEdges.tessLevels.y); // AB
            gl_TessLevelOuter[1] = abs(aEdges.tessLevels.x); // AD
            gl_TessLevelOuter[2] = abs(cEdges.tessLevels.w); // CD
            gl_TessLevelOuter[3] = abs(cEdges.tessLevels.z); // BC
            gl_TessLevelInner[0] = max(abs(aEdges.tessLevels.x), abs(cEdges.tessLevels.z));
            gl_TessLevelInner[1] = max(abs(aEdges.tessLevels.y), abs(cEdges.tessLevels.w));
        }
        
        out_edgeMips = vec4(
            log2(aEdges.uvLengths.y * heightmapLength / aEdges.tessLevels.y) + 1,
            log2(aEdges.uvLengths.x * heightmapLength / aEdges.tessLevels.x) + 1,
            log2(cEdges.uvLengths.w * heightmapLength / cEdges.tessLevels.w) + 1,
            log2(cEdges.uvLengths.z * heightmapLength / cEdges.tessLevels.z) + 1);
        out_cornerMips = vec4(
            calcCornerMips(aEdges), calcCornerMips(bEdges),
            calcCornerMips(cEdges), calcCornerMips(dEdges));
    }
}