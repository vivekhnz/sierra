#version 430
layout(local_size_x = 1) in;

layout(std430, binding = 0) buffer tessellationLevelBuffer
{
    vec4 vertEdgeTessLevels[];
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

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};

uniform int horizontalEdgeCount;
uniform int columnCount;
uniform float targetTriangleSize;
uniform float terrainHeight;
layout(binding = 0) uniform sampler2D heightmapTexture;
layout(binding = 5) uniform sampler2D previewTexture;

vec3 worldToScreen(vec3 p)
{
    vec4 clipPos = camera_transform * vec4(p, 1.0f);
    return clipPos.xyz / clipPos.w;
}

float height(vec2 uv)
{
    return texture(previewTexture, uv).x * terrainHeight;
}

float calcTessLevel(Vertex a, Vertex b)
{
    bool cull = false;

    vec3 pA = vec3(a.pos_x, height(vec2(a.uv_x, a.uv_y)), a.pos_z);
    vec3 pB = vec3(b.pos_x, height(vec2(b.uv_x, b.uv_y)), b.pos_z);
    
    vec3 pAB1 = (pA + pB) * 0.5f;
    vec3 pAB2 = pAB1 + vec3(0.0f, distance(pA, pB), 0.0f);

    float clipW = (camera_transform * vec4(pAB1, 1.0f)).w;
    if (clipW < 0.0f)
    {
        // cull anything behind the camera
        cull = true;
    }
    else
    {
        // cull anything that is too far outside of the screen bounds
        vec3 pAScreen = worldToScreen(pA);
        vec3 pBScreen = worldToScreen(pB);
        float xBounds = min(abs(pAScreen.x), abs(pBScreen.x));
        float yBounds = min(abs(pAScreen.z), abs(pBScreen.z));

        // increase tolerance for triangles directly in front of the camera to avoid them
        // being culled when the camera gets very close
        float tolerance = clipW > 10.0f ? 1.5f : 5.0f;
        if (max(xBounds, yBounds) > tolerance)
        {
            cull = true;
        }
    }
    
    float screenEdgeLength = distance(worldToScreen(pAB1), worldToScreen(pAB2));
    float T = screenEdgeLength / targetTriangleSize;

    /*
     * We could set the tessellation level to 0 to cull patches however we have only
     * determined whether this edge should be culled, not the entire patch.
     *
     * Instead of returning zero, we will negate the tessellation level to indicate that
     * it can be culled, but still provide the calculated tessellation level. This allows
     * the tessellation control shader to only cull the patch if all edges are marked
     * cullable (have a negative tessellation level).
     */
    return T * (cull ? -1.0f : 1.0f);
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
        vertEdgeTessLevels[aIndex].x = T;
        vertEdgeTessLevels[bIndex].z = T;
    }
    else
    {
        uint aIndex = gl_GlobalInvocationID.x - horizontalEdgeCount;
        uint bIndex = aIndex + columnCount;

        Vertex a = vertices[aIndex];
        Vertex b = vertices[bIndex];

        float T = calcTessLevel(a, b);
        vertEdgeTessLevels[aIndex].y = T;
        vertEdgeTessLevels[bIndex].w = T;
    }
}