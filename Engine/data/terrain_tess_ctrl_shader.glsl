#version 430 core
layout(vertices = 4) out;

layout(location = 0) in int in_id[];
layout(location = 1) in vec3 in_worldPos[];
layout(location = 2) in vec2 in_heightmapUV[];

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};

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

vec3 worldToScreen(vec3 p)
{
    vec4 clipPos = camera_transform * vec4(p, 1.0f);
    return clipPos.xyz / clipPos.w;
}

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

        vec2 screenPositions[4] = vec2[4]
        (
            worldToScreen(in_worldPos[0]),
            worldToScreen(in_worldPos[1]),
            worldToScreen(in_worldPos[2]),
            worldToScreen(in_worldPos[3])
        );
        float xBounds = min(
            min(abs(screenPositions[0].x), abs(screenPositions[1].x)),
            min(abs(screenPositions[2].x), abs(screenPositions[3].x))
        );
        float yBounds = min(
            min(abs(screenPositions[0].y), abs(screenPositions[1].y)),
            min(abs(screenPositions[2].y), abs(screenPositions[3].y))
        );
        float cullMult = 1.0f - sign(max(xBounds, yBounds) - 1.5f);
        
        gl_TessLevelOuter[0] = cullMult * aEdges.tessLevels.y; // AB
        gl_TessLevelOuter[1] = cullMult * aEdges.tessLevels.x; // AD
        gl_TessLevelOuter[2] = cullMult * cEdges.tessLevels.w; // CD
        gl_TessLevelOuter[3] = cullMult * cEdges.tessLevels.z; // BC
        gl_TessLevelInner[0] = cullMult * max(aEdges.tessLevels.x, cEdges.tessLevels.z);
        gl_TessLevelInner[1] = cullMult * max(aEdges.tessLevels.y, cEdges.tessLevels.w);
        
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