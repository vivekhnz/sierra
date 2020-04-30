#version 410 core
layout(vertices = 3) out;

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

vec3 calcScreenSpaceEdgeLengths(vec3 a, vec3 b, vec3 c)
{
    vec3 pAB1 = (a + b) * 0.5f;
    vec3 pAB2 = pAB1 + vec3(0.0f, distance(a, b), 0.0f);

    vec3 pBC1 = (b + c) * 0.5f;
    vec3 pBC2 = pBC1 + vec3(0.0f, distance(b, c), 0.0f);

    vec3 pCA1 = (c + a) * 0.5f;
    vec3 pCA2 = pCA1 + vec3(0.0f, distance(c, a), 0.0f);

    return vec3(
        distance(worldToScreen(pAB1), worldToScreen(pAB2)),
        distance(worldToScreen(pBC1), worldToScreen(pBC2)),
        distance(worldToScreen(pCA1), worldToScreen(pCA2)));
}

void main()
{
    out_worldPos[gl_InvocationID] = in_worldPos[gl_InvocationID];
    out_heightmapUV[gl_InvocationID] = in_heightmapUV[gl_InvocationID];
    
    vec3 edgeLengths = calcScreenSpaceEdgeLengths(
        out_worldPos[0], out_worldPos[1], out_worldPos[2]);
    vec3 tessLevels = edgeLengths / targetTriangleSize;
    
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = tessLevels.y; // BC
        gl_TessLevelOuter[1] = tessLevels.z; // CA
        gl_TessLevelOuter[2] = tessLevels.x; // AB
        gl_TessLevelInner[0] = max(max(tessLevels.x, tessLevels.y), tessLevels.z);
    }
}