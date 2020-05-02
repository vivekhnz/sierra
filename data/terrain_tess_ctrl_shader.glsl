#version 410 core
layout(vertices = 4) out;

layout(location = 0) in vec3 in_worldPos[];
layout(location = 1) in vec2 in_heightmapUV[];

uniform mat4 transform;
uniform float targetTriangleSize;

layout(location = 0) out vec3 out_worldPos[];
layout(location = 1) out vec2 out_heightmapUV[];

vec3 worldToScreen(vec3 p)
{
    vec4 clipPos = transform * vec4(p, 1.0f);
    return clipPos.xyz / clipPos.w;
}

vec4 calcScreenSpaceEdgeLengths(vec3 a, vec3 b, vec3 c, vec3 d)
{
    vec3 pAB1 = (a + b) * 0.5f;
    vec3 pAB2 = pAB1 + vec3(0.0f, distance(a, b), 0.0f);

    vec3 pAD1 = (a + d) * 0.5f;
    vec3 pAD2 = pAD1 + vec3(0.0f, distance(a, d), 0.0f);

    vec3 pCD1 = (c + d) * 0.5f;
    vec3 pCD2 = pCD1 + vec3(0.0f, distance(c, d), 0.0f);

    vec3 pBC1 = (b + c) * 0.5f;
    vec3 pBC2 = pBC1 + vec3(0.0f, distance(b, c), 0.0f);

    return vec4(
        distance(worldToScreen(pAB1), worldToScreen(pAB2)),
        distance(worldToScreen(pAD1), worldToScreen(pAD2)),
        distance(worldToScreen(pCD1), worldToScreen(pCD2)),
        distance(worldToScreen(pBC1), worldToScreen(pBC2)));
}

void main()
{
    out_worldPos[gl_InvocationID] = in_worldPos[gl_InvocationID];
    out_heightmapUV[gl_InvocationID] = in_heightmapUV[gl_InvocationID];
    
    vec4 edgeLengths = calcScreenSpaceEdgeLengths(
        out_worldPos[0], out_worldPos[1], out_worldPos[2], out_worldPos[3]);
    vec4 tessLevels = edgeLengths / targetTriangleSize;
    
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = tessLevels.x; // AB
        gl_TessLevelOuter[1] = tessLevels.y; // AD
        gl_TessLevelOuter[2] = tessLevels.z; // CD
        gl_TessLevelOuter[3] = tessLevels.w; // BC
        gl_TessLevelInner[0] = max(tessLevels.y, tessLevels.w);
        gl_TessLevelInner[1] = max(tessLevels.x, tessLevels.z);
    }
}