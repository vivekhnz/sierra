// quad vertex shader
global_variable char *ShaderSrc_QuadVertex = R"glsl(
#version 430 core
layout(location = 0) in vec2 in_mesh_pos;
layout(location = 1) in vec2 in_mesh_uv;
layout(location = 2) in vec4 in_instance_rect;

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};

layout(location = 0) out vec2 out_uv;

void main()
{
    vec2 pos = in_instance_rect.xy + (in_instance_rect.zw * in_mesh_pos);
    gl_Position = camera_transform * vec4(pos, 0, 1);
    out_uv = in_mesh_uv;
}
)glsl";

// textured quad fragment shader
global_variable char *ShaderSrc_TexturedQuadFragment = R"glsl(
#version 430 core
layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D imageTexture;

out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(imageTexture, uv).rgb, 1);
}
)glsl";

// colored quad fragment shader
global_variable char *ShaderSrc_ColoredQuadFragment = R"glsl(
#version 430 core
layout(location = 0) in vec2 uv;

uniform vec3 color;

out vec4 FragColor;

void main()
{
    FragColor = vec4(color, 1);
}
)glsl";

// primitive vertex shader
global_variable char *ShaderSrc_PrimitiveVertex = R"glsl(
#version 430 core
layout(location = 0) in vec3 in_pos;

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};

void main()
{
    gl_Position = camera_transform * vec4(in_pos, 1);
}
)glsl";

// colored primitive fragment shader
global_variable char *ShaderSrc_ColoredPrimitiveFragment = R"glsl(
#version 430 core

uniform vec3 color;

out vec4 FragColor;

void main()
{
    FragColor = vec4(color, 1);
}
)glsl";

// mesh vertex shader
global_variable char *ShaderSrc_MeshVertex = R"glsl(
#version 430 core
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in uint in_instance_id;
layout(location = 3) in mat4 in_instance_transform;

layout(location = 0) out uint out_instance_id;
layout(location = 1) out vec3 out_normal;

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};

void main()
{
    gl_Position = camera_transform * in_instance_transform * vec4(in_pos, 1);
    out_instance_id = in_instance_id;
    mat4 inverseTransposeTransform = transpose(inverse(in_instance_transform));
    out_normal = normalize((inverseTransposeTransform * vec4(in_normal, 0)).xyz);
}
)glsl";

// terrain vertex shader
global_variable char *ShaderSrc_TerrainVertex = R"glsl(
#version 430 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

uniform vec2 terrainOrigin;

layout(location = 0) out int id;
layout(location = 1) out vec3 worldPos;
layout(location = 2) out vec2 heightmapUV;

void main()
{
    id = gl_VertexID;
    worldPos = pos + vec3(terrainOrigin.x, 0, terrainOrigin.y);
    heightmapUV = uv;
}
)glsl";

// terrain tessellation control shader
global_variable char *ShaderSrc_TerrainTessCtrl = R"glsl(
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
        vec4 A = vertEdgeTessLevels[in_id[0]];
        vec4 C = vertEdgeTessLevels[in_id[2]];

        if (A.x < 0 && A.y < 0 && C.z < 0 && C.w < 0)
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
            gl_TessLevelOuter[0] = max(abs(A.y), 1); // AB
            gl_TessLevelOuter[1] = max(abs(A.x), 1); // AD
            gl_TessLevelOuter[2] = max(abs(C.w), 1); // CD
            gl_TessLevelOuter[3] = max(abs(C.z), 1); // BC
            gl_TessLevelInner[0] = max(gl_TessLevelOuter[1], gl_TessLevelOuter[3]);
            gl_TessLevelInner[1] = max(gl_TessLevelOuter[0], gl_TessLevelOuter[2]);
        }
    }
}
)glsl";

// terrain tessellation evaluation shader
global_variable char *ShaderSrc_TerrainTessEval = R"glsl(
#version 430 core
layout(quads, fractional_even_spacing, cw) in;

layout(location = 0) in vec3 in_worldPos[];
layout(location = 1) in vec2 in_heightmapUV[];

layout (std140, binding = 0) uniform Camera
{
    mat4 camera_transform;
};
layout (std140, binding = 1) uniform Lighting
{
    vec4 lighting_lightDir;
    bool lighting_isEnabled;
    bool lighting_isTextureEnabled;
    bool lighting_isNormalMapEnabled;
    bool lighting_isAOMapEnabled;
    bool lighting_isDisplacementMapEnabled;
};

uniform int materialCount;
uniform vec3 terrainDimensions;
uniform vec2 heightmapSize;
uniform vec2 terrainOrigin;
uniform float heightmapOverlapInTexels;

layout(binding = 0) uniform sampler2D activeHeightmapTexture;
layout(binding = 3) uniform sampler2DArray displacementTextures;
layout(binding = 5) uniform sampler2D referenceHeightmapTexture;

struct MaterialProperties
{
    vec2 textureSizeInWorldUnits;
    uint albedoTexture_normalTexture;
    uint displacementTexture_aoTexture;
    vec4 rampParams;
};
layout(std430, binding = 1) buffer materialPropsBuffer
{
    MaterialProperties materialProps[];
};

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 texcoord;
layout(location = 2) out vec2 heights;

vec3 lerp3D(vec3 a, vec3 b, vec3 c, vec3 d)
{
    return mix(mix(a, d, gl_TessCoord.x), mix(b, c, gl_TessCoord.x), gl_TessCoord.y);
}
vec2 lerp2D(vec2 a, vec2 b, vec2 c, vec2 d)
{
    return mix(mix(a, d, gl_TessCoord.x), mix(b, c, gl_TessCoord.x), gl_TessCoord.y);
}
float getDisplacement(vec2 uv, uint textureId, float mip)
{
    vec3 uv3 = vec3(uv, textureId);
    return mix(
        textureLod(displacementTextures, uv3, floor(mip)).x,
        textureLod(displacementTextures, uv3, ceil(mip)).x,
        fract(mip));
}
float calcHeight(vec2 uv, sampler2D heightmapTexture)
{
    vec2 innerHeightmapSize = heightmapSize - (2 * heightmapOverlapInTexels);
    vec2 uvT = ((innerHeightmapSize * uv) + (heightmapOverlapInTexels + 0.5f)) / heightmapSize;
    return texture(heightmapTexture, uvT).x;
}
float height(vec2 uv)
{
    return calcHeight(uv, activeHeightmapTexture);
}
float refHeight(vec2 uv)
{
    return calcHeight(uv, referenceHeightmapTexture);
}
vec3 calcTriplanarBlend(vec3 normal)
{
    // bias towards Y-axis
    vec3 blend = vec3(pow(abs(normal.x), 6), pow(abs(normal.y), 1), pow(abs(normal.z), 6));
    blend = normalize(max(blend, 0.00001));
    blend /= blend.x + blend.y + blend.z;
    return blend;
}
vec3 triplanar3D(vec3 xVal, vec3 yVal, vec3 zVal, vec3 blend)
{
    return (xVal * blend.x) + (yVal * blend.y) + (zVal * blend.z);
}

void main()
{
    // calculate normal
    vec2 hUV = lerp2D(in_heightmapUV[0], in_heightmapUV[1], in_heightmapUV[2], in_heightmapUV[3]);
    float altitude = height(hUV);
    float normalSampleOffsetInTexels = 7;
    vec2 normalSampleOffsetInUvCoords = normalSampleOffsetInTexels / heightmapSize;
    float hL = height(vec2(hUV.x - normalSampleOffsetInUvCoords.x, hUV.y));
    float hR = height(vec2(hUV.x + normalSampleOffsetInUvCoords.x, hUV.y));
    float hD = height(vec2(hUV.x, hUV.y - normalSampleOffsetInUvCoords.x));
    float hU = height(vec2(hUV.x, hUV.y + normalSampleOffsetInUvCoords.y));
    
    float nY = (2 * terrainDimensions.x * normalSampleOffsetInUvCoords.x) / terrainDimensions.y;
    normal = normalize(vec3(hL - hR, nY, hU - hD));
    float slope = 1 - normal.y;
    
    // calculate texture coordinates
    vec3 triBlend = calcTriplanarBlend(normal);
    vec3 triAxisSign = sign(normal);
    texcoord = vec3(
        terrainOrigin.x + (hUV.x * terrainDimensions.x),
        -altitude * terrainDimensions.y,
        terrainOrigin.y + (hUV.y * terrainDimensions.z));

    vec2 baseTexcoordsX = vec2(texcoord.z * triAxisSign.x, texcoord.y);
    vec2 baseTexcoordsY = vec2(texcoord.x * triAxisSign.y, texcoord.z);
    vec2 baseTexcoordsZ = vec2(texcoord.x * triAxisSign.z, texcoord.y);
    
    vec3 pos = lerp3D(in_worldPos[0], in_worldPos[1], in_worldPos[2], in_worldPos[3]);
    pos.y = altitude * terrainDimensions.y;

    heights.x = refHeight(hUV);
    heights.y = altitude;

    if (lighting_isDisplacementMapEnabled)
    {
        vec3 displacement = vec3(0);
        for (int i = 0; i < materialCount; i++)
        {
            vec2 textureSizeInWorldUnits = materialProps[i].textureSizeInWorldUnits;

            vec2 mUVX = baseTexcoordsX / textureSizeInWorldUnits.yy;
            vec2 mUVY = baseTexcoordsY / textureSizeInWorldUnits.xy;
            vec2 mUVZ = baseTexcoordsZ / textureSizeInWorldUnits.xy;

            float scaledMip = log2(terrainDimensions.x / textureSizeInWorldUnits.x);
            uint displacementTexture = materialProps[i].displacementTexture_aoTexture >> 16;
            vec3 currentLayerDisplacement = triplanar3D(
                vec3(getDisplacement(mUVX, displacementTexture, scaledMip) * -triAxisSign.x, 0, 0),
                vec3(0, getDisplacement(mUVY, displacementTexture, scaledMip) * triAxisSign.y, 0),
                vec3(0, 0, getDisplacement(mUVZ, displacementTexture, scaledMip) * triAxisSign.z),
                triBlend);
            
            if (i == 0)
            {
                displacement = currentLayerDisplacement;
            }
            else
            {
                vec4 ramp = materialProps[i].rampParams;
                float blendAmount = clamp((slope - ramp.x) / (ramp.y - ramp.x), 0, 1);
                blendAmount *= clamp((altitude - ramp.z) / (ramp.w - ramp.z), 0, 1);
                displacement = mix(displacement, currentLayerDisplacement, blendAmount);
            }
        }

        pos += displacement * 0.8;
    }
    
    gl_Position = camera_transform * vec4(pos, 1.0f);
}
)glsl";

// terrain calculate tessellation levels compute shader
global_variable char *ShaderSrc_TerrainCalcTessLevel = R"glsl(
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
uniform int vertsPerEdge;
uniform float targetTriangleSize;
uniform float terrainHeight;
uniform vec2 terrainOrigin;
uniform vec2 heightmapSize;
uniform float heightmapOverlapInTexels;
layout(binding = 0) uniform sampler2D heightmapTexture;

vec3 worldToScreen(vec3 p)
{
    vec4 clipPos = camera_transform * vec4(p, 1.0f);
    return clipPos.xyz / clipPos.w;
}
float calcHeight(vec2 uv, sampler2D heightmapTexture)
{
    vec2 innerHeightmapSize = heightmapSize - (2 * heightmapOverlapInTexels);
    vec2 uvT = ((innerHeightmapSize * uv) + (heightmapOverlapInTexels + 0.5f)) / heightmapSize;
    return texture(heightmapTexture, uvT).x;
}
float height(vec2 uv)
{
    return calcHeight(uv, heightmapTexture).x * terrainHeight;
}

float calcTessLevel(Vertex a, Vertex b)
{
    bool cull = false;

    vec3 pA = vec3(a.pos_x + terrainOrigin.x, height(vec2(a.uv_x, a.uv_y)), a.pos_z + terrainOrigin.y);
    vec3 pB = vec3(b.pos_x + terrainOrigin.x, height(vec2(b.uv_x, b.uv_y)), b.pos_z + terrainOrigin.y);
    
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
            + uint(floor(float(gl_GlobalInvocationID.x) / (vertsPerEdge - 1)));
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
        uint bIndex = aIndex + vertsPerEdge;

        Vertex a = vertices[aIndex];
        Vertex b = vertices[bIndex];

        float T = calcTessLevel(a, b);
        vertEdgeTessLevels[aIndex].y = T;
        vertEdgeTessLevels[bIndex].w = T;
    }
}
)glsl";