#version 430
layout(local_size_x = 1) in;

struct VertexEdgeData
{
    vec4 tessLevels;
    vec4 uvLengths;
};
layout(std430, binding = 0) buffer tessellationLevelBuffer
{
    VertexEdgeData vertEdgeData[];
};

struct Vertex
{
    float pos_x;
    float pos_y;
    float pos_z;
    float uv_x;
    float uv_y;
};
layout(std430, binding = 1) buffer vertexBuffer
{
    Vertex vertices[];
};

uniform int horizontalEdgeCount;
uniform int columnCount;
uniform float targetTriangleSize;
uniform mat4 transform;

vec3 worldToScreen(vec3 p)
{
    vec4 clipPos = transform * vec4(p, 1.0f);
    return clipPos.xyz / clipPos.w;
}

float calcTessLevel(Vertex a, Vertex b)
{
    vec3 pA = vec3(a.pos_x, a.pos_y, a.pos_z);
    vec3 pB = vec3(b.pos_x, b.pos_y, b.pos_z);
    
    vec3 pAB1 = (pA + pB) * 0.5f;
    vec3 pAB2 = pAB1 + vec3(0.0f, distance(pA, pB), 0.0f);
    
    float screenEdgeLength = distance(worldToScreen(pAB1), worldToScreen(pAB2));
    return screenEdgeLength / targetTriangleSize;
}

void main()
{
    if (gl_GlobalInvocationID.x < horizontalEdgeCount)
    {
        uint aIndex = gl_GlobalInvocationID.x
            + uint(floor(float(gl_GlobalInvocationID.x) / (columnCount - 1)));
        uint bIndex = aIndex + 1;

        Vertex a = vertices[aIndex];
        Vertex b = vertices[bIndex];

        float T = calcTessLevel(a, b);
        vertEdgeData[aIndex].tessLevels.x = T;
        vertEdgeData[bIndex].tessLevels.z = T;

        float L = length(vec2(a.uv_x - b.uv_x, a.uv_y - b.uv_y));
        vertEdgeData[aIndex].uvLengths.x = L;
        vertEdgeData[bIndex].uvLengths.z = L;
    }
    else
    {
        uint aIndex = gl_GlobalInvocationID.x - horizontalEdgeCount;
        uint bIndex = aIndex + columnCount;

        Vertex a = vertices[aIndex];
        Vertex b = vertices[bIndex];

        float T = calcTessLevel(a, b);
        vertEdgeData[aIndex].tessLevels.y = T;
        vertEdgeData[bIndex].tessLevels.w = T;

        float L = length(vec2(a.uv_x - b.uv_x, a.uv_y - b.uv_y));
        vertEdgeData[aIndex].uvLengths.y = L;
        vertEdgeData[bIndex].uvLengths.w = L;
    }
}