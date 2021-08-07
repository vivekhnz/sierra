#include "engine_render_backend.h"

extern EnginePlatformApi Platform;
global_variable bool WasRendererReloaded = true;

#define OPENGL_UBO_SLOT_CAMERA 0
#define OPENGL_UBO_SLOT_LIGHTING 1

struct OpenGlInternalShaders
{
    bool initialized;

    uint32 quadVertexShaderId;
    uint32 texturedQuadFragmentShaderId;
    uint32 coloredQuadFragmentShaderId;

    uint32 texturedQuadShaderProgramId;
    uint32 coloredQuadShaderProgramId;

    uint32 meshVertexShaderId;

    uint32 terrainVertexShaderId;
    uint32 terrainTessCtrlShaderId;
    uint32 terrainTessEvalShaderId;
    uint32 terrainCalcTessLevelShaderId;
    uint32 terrainCalcTessLevelShaderProgramId;
};

struct OpenGlRenderContext
{
    // access this via getInternalShaders which handles the engine DLL being reloaded
    OpenGlInternalShaders _shaders;

    uint32 globalVertexArrayId;

    uint32 cameraUniformBufferId;
    uint32 lightingUniformBufferId;

    uint32 quadElementBufferId;
    uint32 quadTopDownVertexBufferId;
    uint32 quadBottomUpVertexBufferId;

    uint32 quadInstanceBufferId;
    uint32 meshInstanceBufferId;
};
struct OpenGlCameraState
{
    glm::mat4 transform;
};
struct OpenGlLightingState
{
    glm::vec4 lightDir;

    // todo: pack these into a single uint8
    uint32 isEnabled;
    uint32 isTextureEnabled;
    uint32 isNormalMapEnabled;
    uint32 isAOMapEnabled;
    uint32 isDisplacementMapEnabled;
};

struct OpenGlMesh
{
    uint32 vertexBufferId;
    uint32 elementBufferId;
    uint32 elementCount;
};
struct OpenGlTextureDescriptor
{
    uint32 elementType;
    uint32 elementSize;
    uint32 cpuFormat;
    uint32 gpuFormat;
    bool isInteger;
};
struct OpenGlRenderTarget
{
    RenderTarget extTarget;

    OpenGlTextureDescriptor descriptor;
    bool hasDepthBuffer;
    uint32 framebufferId;
};

RenderBackendContext initializeRenderBackend(MemoryArena *arena)
{
    OpenGlRenderContext *ctx = pushStruct(arena, OpenGlRenderContext);
    *ctx = {};

    glGenVertexArrays(1, &ctx->globalVertexArrayId);

    // initialize camera state
    glGenBuffers(1, &ctx->cameraUniformBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, ctx->cameraUniformBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(OpenGlCameraState), 0, GL_DYNAMIC_DRAW);
    glBindBufferRange(
        GL_UNIFORM_BUFFER, OPENGL_UBO_SLOT_CAMERA, ctx->cameraUniformBufferId, 0, sizeof(OpenGlCameraState));

    // initialize lighting state
    glGenBuffers(1, &ctx->lightingUniformBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, ctx->lightingUniformBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(OpenGlLightingState), 0, GL_DYNAMIC_DRAW);
    glBindBufferRange(
        GL_UNIFORM_BUFFER, OPENGL_UBO_SLOT_LIGHTING, ctx->lightingUniformBufferId, 0, sizeof(OpenGlLightingState));

    // create quad buffers
    float quadTopDownVerts[16] = {
        0, 0, 0, 0, //
        1, 0, 1, 0, //
        1, 1, 1, 1, //
        0, 1, 0, 1  //
    };
    glGenBuffers(1, &ctx->quadTopDownVertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->quadTopDownVertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadTopDownVerts), quadTopDownVerts, GL_STATIC_DRAW);

    float quadBottomUpVerts[16] = {
        0, 0, 0, 1, //
        1, 0, 1, 1, //
        1, 1, 1, 0, //
        0, 1, 0, 0  //
    };
    glGenBuffers(1, &ctx->quadBottomUpVertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->quadBottomUpVertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadBottomUpVerts), quadBottomUpVerts, GL_STATIC_DRAW);

    uint32 quadIndices[6] = {0, 1, 2, 0, 2, 3};
    glGenBuffers(1, &ctx->quadElementBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->quadElementBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glGenBuffers(1, &ctx->quadInstanceBufferId);
    glGenBuffers(1, &ctx->meshInstanceBufferId);

    return {ctx};
}

bool createOpenGlShader(uint32 type, char *src, uint32 *out_id)
{
    uint32 id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);

    glCompileShader(id);
    int32 succeeded;
    glGetShaderiv(id, GL_COMPILE_STATUS, &succeeded);
    if (succeeded)
    {
        *out_id = id;
        return 1;
    }
    else
    {
        char infoLog[512];
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        Platform.logMessage(infoLog);

        return 0;
    }
}

bool createOpenGlShaderProgram(uint32 shaderCount, uint32 *shaderIds, uint32 *out_id)
{
    uint32 id = glCreateProgram();
    for (uint32 i = 0; i < shaderCount; i++)
    {
        glAttachShader(id, shaderIds[i]);
    }

    glLinkProgram(id);
    int32 succeeded;
    glGetProgramiv(id, GL_LINK_STATUS, &succeeded);
    if (succeeded)
    {
        for (uint32 i = 0; i < shaderCount; i++)
        {
            glDetachShader(id, shaderIds[i]);
        }
        *out_id = id;
        return 1;
    }
    else
    {
        char infoLog[512];
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        Platform.logMessage(infoLog);

        return 0;
    }
}

uint32 getShaderProgramId(ShaderHandle handle)
{
    uint64 id = (uint64)handle.ptr;
    assert(id < UINT32_MAX);
    return (uint32)id;
}
ShaderHandle getShaderHandle(uint32 programId)
{
    ShaderHandle result;
    result.ptr = (void *)((uint64)programId);
    return result;
}

OpenGlInternalShaders *getInternalShaders(OpenGlRenderContext *ctx)
{
    OpenGlInternalShaders *shaders = &ctx->_shaders;
    if (WasRendererReloaded)
    {
        if (shaders->initialized)
        {
            glDeleteProgram(shaders->texturedQuadShaderProgramId);
            glDeleteProgram(shaders->coloredQuadShaderProgramId);
            glDeleteProgram(shaders->terrainCalcTessLevelShaderProgramId);
            glDeleteShader(shaders->quadVertexShaderId);
            glDeleteShader(shaders->texturedQuadFragmentShaderId);
            glDeleteShader(shaders->coloredQuadFragmentShaderId);
            glDeleteShader(shaders->meshVertexShaderId);
            glDeleteShader(shaders->terrainVertexShaderId);
            glDeleteShader(shaders->terrainTessCtrlShaderId);
            glDeleteShader(shaders->terrainTessEvalShaderId);
            glDeleteShader(shaders->terrainCalcTessLevelShaderId);
        }

        // create quad shader programs
        char *quadVertexShaderSrc = R"(
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
    )";
        assert(createOpenGlShader(GL_VERTEX_SHADER, quadVertexShaderSrc, &shaders->quadVertexShaderId));

        char *texturedQuadFragmentShaderSrc = R"(
#version 430 core
layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D imageTexture;

out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(imageTexture, uv).rgb, 1);
}
    )";
        assert(createOpenGlShader(
            GL_FRAGMENT_SHADER, texturedQuadFragmentShaderSrc, &shaders->texturedQuadFragmentShaderId));
        uint32 texturedQuadShaderIds[] = {shaders->quadVertexShaderId, shaders->texturedQuadFragmentShaderId};
        assert(createOpenGlShaderProgram(2, texturedQuadShaderIds, &shaders->texturedQuadShaderProgramId));

        char *coloredQuadFragmentShaderSrc = R"(
#version 430 core
layout(location = 0) in vec2 uv;

uniform vec3 color;

out vec4 FragColor;

void main()
{
    FragColor = vec4(color, 1);
}
    )";
        assert(createOpenGlShader(
            GL_FRAGMENT_SHADER, coloredQuadFragmentShaderSrc, &shaders->coloredQuadFragmentShaderId));
        uint32 coloredQuadShaderIds[] = {shaders->quadVertexShaderId, shaders->coloredQuadFragmentShaderId};
        assert(createOpenGlShaderProgram(2, coloredQuadShaderIds, &shaders->coloredQuadShaderProgramId));

        // create mesh vertex shader
        char *meshVertexShaderSrc = R"(
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
    )";
        assert(createOpenGlShader(GL_VERTEX_SHADER, meshVertexShaderSrc, &shaders->meshVertexShaderId));

        // create terrain shaders
        char *terrainVertexShaderSrc = R"(
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
    )";
        char *terrainTessCtrlShaderSrc = R"(
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
    )";
        char *terrainTessEvalShaderSrc = R"(
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

layout(binding = 0) uniform sampler2D activeHeightmapTexture;
layout(binding = 3) uniform sampler2DArray displacementTextures;
layout(binding = 5) uniform sampler2D referenceHeightmapTexture;
layout(binding = 6) uniform sampler2D xAdjacentActiveHeightmapTexture;
layout(binding = 7) uniform sampler2D xAdjacentReferenceHeightmapTexture;
layout(binding = 8) uniform sampler2D yAdjacentActiveHeightmapTexture;
layout(binding = 9) uniform sampler2D yAdjacentReferenceHeightmapTexture;
layout(binding = 10) uniform sampler2D oppositeActiveHeightmapTexture;
layout(binding = 11) uniform sampler2D oppositeReferenceHeightmapTexture;

struct MaterialProperties
{
    vec2 textureSizeInWorldUnits;
    vec2 _padding;
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
float getDisplacement(vec2 uv, int layerIdx, float mip)
{
    vec3 uvLayered = vec3(uv, layerIdx);
    return mix(
        textureLod(displacementTextures, uvLayered, floor(mip)).x,
        textureLod(displacementTextures, uvLayered, ceil(mip)).x,
        fract(mip));
}
float calcHeight(vec2 uv, sampler2D thisTileTex,
    sampler2D xAdjacentTex, sampler2D yAdjacentTex, sampler2D oppositeTex)
{
    vec2 minUV = 1 / (2 * heightmapSize);
    vec2 maxUV = 1 - minUV;
    vec2 uvClamped = min(uv + minUV, maxUV);
    float result = texture(thisTileTex, uvClamped).x;

    vec2 adjBlend = clamp((uv + (2 * minUV) - 1) / (2 * minUV), 0, 1);
    if (adjBlend.x > 0 && adjBlend.y > 0)
    {
        result = texture(oppositeTex, clamp(uv + minUV - 1, minUV, maxUV)).x;
    }
    else
    {
        vec2 adjUV = clamp(uv + minUV - 1, minUV, maxUV);
        float adjXHeight = texture(xAdjacentTex, vec2(adjUV.x, uvClamped.y)).x;
        float adjYHeight = texture(yAdjacentTex, vec2(uvClamped.x, adjUV.y)).x;
        result = mix(mix(result, adjXHeight, adjBlend.x), adjYHeight, adjBlend.y);
    }
    return result;
}
float height(vec2 uv)
{
    return calcHeight(uv, activeHeightmapTexture,
        xAdjacentActiveHeightmapTexture, yAdjacentActiveHeightmapTexture, oppositeActiveHeightmapTexture);
}
float refHeight(vec2 uv)
{
    return calcHeight(uv, referenceHeightmapTexture,
        xAdjacentReferenceHeightmapTexture, yAdjacentReferenceHeightmapTexture,
        oppositeReferenceHeightmapTexture);
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
    float normalSampleOffsetInTexels = 8;
    vec2 normalSampleOffsetInUvCoords = normalSampleOffsetInTexels / heightmapSize;
    float hL = height(vec2(hUV.x, hUV.y));
    float hR = height(vec2(hUV.x + normalSampleOffsetInUvCoords.x, hUV.y));
    float hD = height(vec2(hUV.x, hUV.y));
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

            vec2 materialTexcoordsX = baseTexcoordsX / textureSizeInWorldUnits.yy;
            vec2 materialTexcoordsY = baseTexcoordsY / textureSizeInWorldUnits.xy;
            vec2 materialTexcoordsZ = baseTexcoordsZ / textureSizeInWorldUnits.xy;

            float scaledMip = log2(terrainDimensions.x / textureSizeInWorldUnits.x);
            vec3 currentLayerDisplacement = triplanar3D(
                vec3(getDisplacement(materialTexcoordsX, i, scaledMip) * -triAxisSign.x, 0, 0),
                vec3(0, getDisplacement(materialTexcoordsY, i, scaledMip) * triAxisSign.y, 0),
                vec3(0, 0, getDisplacement(materialTexcoordsZ, i, scaledMip) * triAxisSign.z),
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
        )";
        char *terrainCalcTessLevelShaderSrc = R"(
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
uniform vec2 terrainOrigin;
uniform vec2 heightmapSize;
layout(binding = 0) uniform sampler2D heightmapTexture;
layout(binding = 6) uniform sampler2D xAdjacentHeightmapTexture;
layout(binding = 8) uniform sampler2D yAdjacentHeightmapTexture;
layout(binding = 10) uniform sampler2D oppositeHeightmapTexture;

vec3 worldToScreen(vec3 p)
{
    vec4 clipPos = camera_transform * vec4(p, 1.0f);
    return clipPos.xyz / clipPos.w;
}
float calcHeight(vec2 uv, sampler2D thisTileTex,
    sampler2D xAdjacentTex, sampler2D yAdjacentTex, sampler2D oppositeTex)
{
    vec2 minUV = 1 / (2 * heightmapSize);
    vec2 maxUV = 1 - minUV;
    vec2 uvClamped = min(uv + minUV, maxUV);
    float result = texture(thisTileTex, uvClamped).x;

    vec2 adjBlend = clamp((uv + (2 * minUV) - 1) / (2 * minUV), 0, 1);
    if (adjBlend.x > 0 && adjBlend.y > 0)
    {
        result = texture(oppositeTex, clamp(uv + minUV - 1, minUV, maxUV)).x;
    }
    else
    {
        vec2 adjUV = clamp(uv + minUV - 1, minUV, maxUV);
        float adjXHeight = texture(xAdjacentTex, vec2(adjUV.x, uvClamped.y)).x;
        float adjYHeight = texture(yAdjacentTex, vec2(uvClamped.x, adjUV.y)).x;
        result = mix(mix(result, adjXHeight, adjBlend.x), adjYHeight, adjBlend.y);
    }
    return result;
}
float height(vec2 uv)
{
    return calcHeight(uv, heightmapTexture, xAdjacentHeightmapTexture, yAdjacentHeightmapTexture,
        oppositeHeightmapTexture).x * terrainHeight;
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
        )";

        assert(createOpenGlShader(GL_VERTEX_SHADER, terrainVertexShaderSrc, &shaders->terrainVertexShaderId));
        assert(createOpenGlShader(
            GL_TESS_CONTROL_SHADER, terrainTessCtrlShaderSrc, &shaders->terrainTessCtrlShaderId));
        assert(createOpenGlShader(
            GL_TESS_EVALUATION_SHADER, terrainTessEvalShaderSrc, &shaders->terrainTessEvalShaderId));
        assert(createOpenGlShader(
            GL_COMPUTE_SHADER, terrainCalcTessLevelShaderSrc, &shaders->terrainCalcTessLevelShaderId));
        assert(createOpenGlShaderProgram(
            1, &shaders->terrainCalcTessLevelShaderId, &shaders->terrainCalcTessLevelShaderProgramId));

        shaders->initialized = true;
        WasRendererReloaded = false;
    }
    return shaders;
}

bool createShader(RenderBackendContext rctx, ShaderType type, char *src, ShaderHandle *out_handle)
{
    OpenGlRenderContext *ctx = (OpenGlRenderContext *)rctx.ptr;
    bool result = false;
    OpenGlInternalShaders *shaders = getInternalShaders(ctx);

    uint32 shaderIds[4];
    uint32 shaderCount = 0;

    switch (type)
    {
    case SHADER_TYPE_QUAD:
        shaderIds[0] = shaders->quadVertexShaderId;
        shaderCount = 2;
        break;
    case SHADER_TYPE_MESH:
        shaderIds[0] = shaders->meshVertexShaderId;
        shaderCount = 2;
        break;
    case SHADER_TYPE_TERRAIN:
        shaderIds[0] = shaders->terrainVertexShaderId;
        shaderIds[1] = shaders->terrainTessCtrlShaderId;
        shaderIds[2] = shaders->terrainTessEvalShaderId;
        shaderCount = 4;
        break;
    }
    assert(shaderCount);

    if (createOpenGlShader(GL_FRAGMENT_SHADER, src, &shaderIds[shaderCount - 1]))
    {
        uint32 programId;
        if (createOpenGlShaderProgram(shaderCount, shaderIds, &programId))
        {
            *out_handle = getShaderHandle(programId);
            result = true;
        }
    }

    return result;
}
void destroyShader(ShaderHandle handle)
{
    uint32 id = getShaderProgramId(handle);
    glDeleteProgram(id);
}

ShaderHandle getTexturedQuadShader(RenderBackendContext rctx)
{
    OpenGlRenderContext *ctx = (OpenGlRenderContext *)rctx.ptr;
    OpenGlInternalShaders *shaders = getInternalShaders(ctx);
    return getShaderHandle(shaders->texturedQuadShaderProgramId);
}
ShaderHandle getColoredQuadShader(RenderBackendContext rctx)
{
    OpenGlRenderContext *ctx = (OpenGlRenderContext *)rctx.ptr;
    OpenGlInternalShaders *shaders = getInternalShaders(ctx);
    return getShaderHandle(shaders->coloredQuadShaderProgramId);
}

MeshHandle createMesh(MemoryArena *arena, void *vertices, uint32 vertexCount, void *indices, uint32 indexCount)
{
    OpenGlMesh *result = pushStruct(arena, OpenGlMesh);
    result->elementCount = indexCount;

    uint32 vertexBufferStride = 6 * sizeof(float);
    glGenBuffers(1, &result->vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, result->vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexBufferStride, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &result->elementBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result->elementBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(uint32), indices, GL_STATIC_DRAW);

    return {result};
}
void destroyMesh(MeshHandle handle)
{
    OpenGlMesh *mesh = (OpenGlMesh *)handle.ptr;
    glDeleteBuffers(1, &mesh->vertexBufferId);
    glDeleteBuffers(1, &mesh->elementBufferId);
}

uint32 getTextureId(TextureHandle handle)
{
    uint64 packed = (uint64)handle.ptr;
    uint32 id = (uint32)packed;
    return id;
}
TextureFormat getTextureFormat(TextureHandle handle)
{
    uint64 packed = (uint64)handle.ptr;
    TextureFormat format = (TextureFormat)(packed >> 32);
    return format;
}
TextureHandle getTextureHandle(uint32 id, TextureFormat format)
{
    assert(format < UINT32_MAX);

    TextureHandle result;
    uint64 packed = ((uint64)format << 32) | id;
    result.ptr = (void *)packed;
    return result;
}
OpenGlTextureDescriptor getTextureDescriptor(TextureFormat format)
{
    OpenGlTextureDescriptor result = {};
    if (format == TEXTURE_FORMAT_RGB8)
    {
        result.elementType = GL_UNSIGNED_BYTE;
        result.elementSize = sizeof(uint8);
        result.cpuFormat = GL_RGB;
        result.gpuFormat = GL_RGB;
        result.isInteger = false;
    }
    else if (format == TEXTURE_FORMAT_R16)
    {
        result.elementType = GL_UNSIGNED_SHORT;
        result.elementSize = sizeof(uint16);
        result.cpuFormat = GL_R16;
        result.gpuFormat = GL_RED;
        result.isInteger = false;
    }
    else if (format == TEXTURE_FORMAT_R8UI)
    {
        result.elementType = GL_UNSIGNED_BYTE;
        result.elementSize = sizeof(uint8);
        result.cpuFormat = GL_R8UI;
        result.gpuFormat = GL_RED_INTEGER;
        result.isInteger = true;
    }
    else if (format == TEXTURE_FORMAT_R16UI)
    {
        result.elementType = GL_UNSIGNED_SHORT;
        result.elementSize = sizeof(uint16);
        result.cpuFormat = GL_R16UI;
        result.gpuFormat = GL_RED_INTEGER;
        result.isInteger = true;
    }
    else if (format == TEXTURE_FORMAT_R32UI)
    {
        result.elementType = GL_UNSIGNED_INT;
        result.elementSize = sizeof(uint32);
        result.cpuFormat = GL_R32UI;
        result.gpuFormat = GL_RED_INTEGER;
        result.isInteger = true;
    }
    else
    {
        assert(!"Unknown texture format");
    }

    return result;
}

TextureHandle createTexture(uint32 width, uint32 height, TextureFormat format)
{
    uint32 id = 0;
    glGenTextures(1, &id);
    OpenGlTextureDescriptor descriptor = getTextureDescriptor(format);

    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float black[] = {0, 0, 0, 0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, black);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, descriptor.isInteger ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, descriptor.isInteger ? GL_NEAREST : GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D, 0, descriptor.cpuFormat, width, height, 0, descriptor.gpuFormat, descriptor.elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    return getTextureHandle(id, format);
}
void updateTexture(TextureHandle handle, uint32 width, uint32 height, void *pixels)
{
    uint32 id = getTextureId(handle);
    TextureFormat format = getTextureFormat(handle);
    OpenGlTextureDescriptor descriptor = getTextureDescriptor(format);

    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, descriptor.cpuFormat, width, height, 0, descriptor.gpuFormat,
        descriptor.elementType, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
}
void *getPixels(MemoryArena *arena, TextureHandle handle, uint32 width, uint32 height, uint32 *out_pixelCount)
{
    uint32 id = getTextureId(handle);
    TextureFormat format = getTextureFormat(handle);
    OpenGlTextureDescriptor descriptor = getTextureDescriptor(format);

    uint32 pixelCount = width * height;
    uint32 bufferSize = pixelCount * descriptor.elementSize;
    void *buffer = pushSize(arena, bufferSize);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, descriptor.gpuFormat, descriptor.elementType, buffer);

    *out_pixelCount = pixelCount;
    return buffer;
}
void *getPixelsInRegion(MemoryArena *arena,
    TextureHandle handle,
    uint32 x,
    uint32 y,
    uint32 width,
    uint32 height,
    uint32 *out_pixelCount)
{
    uint32 id = getTextureId(handle);
    TextureFormat format = getTextureFormat(handle);
    OpenGlTextureDescriptor descriptor = getTextureDescriptor(format);

    uint32 pixelCount = width * height;
    uint32 bufferSize = pixelCount * descriptor.elementSize;
    void *buffer = pushSize(arena, bufferSize);

    glGetTextureSubImage(
        id, 0, x, y, 0, width, height, 1, descriptor.gpuFormat, descriptor.elementType, bufferSize, buffer);

    *out_pixelCount = pixelCount;
    return buffer;
}

RenderTarget *createRenderTarget(
    MemoryArena *arena, uint32 width, uint32 height, TextureFormat format, bool createDepthBuffer)
{
    OpenGlRenderTarget *internalTarget = pushStruct(arena, OpenGlRenderTarget);
    RenderTarget *target = &internalTarget->extTarget;
    target->width = width;
    target->height = height;

    internalTarget->descriptor = getTextureDescriptor(format);
    internalTarget->hasDepthBuffer = createDepthBuffer;

    // create target texture
    uint32 textureId;
    glGenTextures(1, &textureId);
    target->textureHandle = getTextureHandle(textureId, format);

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float black[] = {0, 0, 0, 0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, black);

    uint32 filterMode = internalTarget->descriptor.isInteger ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexImage2D(GL_TEXTURE_2D, 0, internalTarget->descriptor.cpuFormat, width, height, 0,
        internalTarget->descriptor.gpuFormat, internalTarget->descriptor.elementType, 0);

    // create depth buffer
    uint32 depthTextureId;
    if (createDepthBuffer)
    {
        glGenTextures(1, &depthTextureId);
        target->depthTextureHandle = getTextureHandle(depthTextureId, (TextureFormat)0);

        glBindTexture(GL_TEXTURE_2D, depthTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, black);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }

    // create framebuffer
    glGenFramebuffers(1, &internalTarget->framebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, internalTarget->framebufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
    if (createDepthBuffer)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextureId, 0);
    }
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return target;
}
void resizeRenderTarget(RenderTarget *target, uint32 width, uint32 height)
{
    OpenGlRenderTarget *internalTarget = (OpenGlRenderTarget *)target;
    OpenGlTextureDescriptor *descriptor = &internalTarget->descriptor;

    glBindTexture(GL_TEXTURE_2D, getTextureId(target->textureHandle));
    glTexImage2D(GL_TEXTURE_2D, 0, descriptor->cpuFormat, width, height, 0, descriptor->gpuFormat,
        descriptor->elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (internalTarget->hasDepthBuffer)
    {
        glBindTexture(GL_TEXTURE_2D, getTextureId(target->depthTextureHandle));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }
}

bool applyEffect(RenderEffect *effect)
{
    bool isMissingResources = true;
    if (effect->shaderHandle.ptr != 0)
    {
        isMissingResources = false;

        uint32 shaderProgramId = getShaderProgramId(effect->shaderHandle);
        glUseProgram(shaderProgramId);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        switch (effect->blendMode)
        {
        case EFFECT_BLEND_ALPHA_BLEND:
        {
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        break;
        case EFFECT_BLEND_ADDITIVE:
        {
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);
        }
        break;
        case EFFECT_BLEND_MAX:
        {
            glBlendEquation(GL_MAX);
            glBlendFunc(GL_ONE, GL_ONE);
        }
        break;
        }

        RenderEffectParameter *effectParam = effect->firstParameter;
        while (effectParam)
        {
            uint32 loc = glGetUniformLocation(shaderProgramId, effectParam->name);
            switch (effectParam->type)
            {
            case EFFECT_PARAM_TYPE_FLOAT:
                glProgramUniform1f(shaderProgramId, loc, effectParam->value.f);
                break;
            case EFFECT_PARAM_TYPE_VEC3:
                glProgramUniform3fv(shaderProgramId, loc, 1, glm::value_ptr(effectParam->value.v3));
                break;
            case EFFECT_PARAM_TYPE_INT:
                glProgramUniform1i(shaderProgramId, loc, effectParam->value.i);
                break;
            case EFFECT_PARAM_TYPE_UINT:
                glProgramUniform1ui(shaderProgramId, loc, effectParam->value.u);
                break;
            default:
                assert(!"Invalid effect parameter type");
                break;
            }

            effectParam = effectParam->next;
        }

        RenderEffectTexture *effectTexture = effect->firstTexture;
        while (effectTexture)
        {
            glActiveTexture(GL_TEXTURE0 + effectTexture->slot);
            glBindTexture(GL_TEXTURE_2D, getTextureId(effectTexture->handle));

            effectTexture = effectTexture->next;
        }
    }

    return !isMissingResources;
}
bool drawToTarget(DispatchedRenderQueue *rq, uint32 width, uint32 height, RenderTarget *target)
{
    OpenGlRenderContext *ctx = (OpenGlRenderContext *)rq->ctx.ptr;
    OpenGlInternalShaders *shaders = getInternalShaders(ctx);

    if (target)
    {
        OpenGlRenderTarget *internalTarget = (OpenGlRenderTarget *)target;
        glBindFramebuffer(GL_FRAMEBUFFER, internalTarget->framebufferId);
    }
    glViewport(0, 0, width, height);

    glBindBuffer(GL_ARRAY_BUFFER, ctx->quadInstanceBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RenderQuad) * rq->quadCount, rq->quads, GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, ctx->meshInstanceBufferId);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(RenderMeshInstance) * rq->meshInstanceCount, rq->meshInstances, GL_STREAM_DRAW);

    glBindVertexArray(ctx->globalVertexArrayId);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);

    bool isMissingResources = false;
    RenderQueueCommandHeader *command = rq->firstCommand;
    while (command)
    {
        void *commandData = command + 1;

        switch (command->type)
        {
        case RENDER_CMD_SetCameraCommand:
        {
            SetCameraCommand *cmd = (SetCameraCommand *)commandData;

            OpenGlCameraState camera = {};
            if (cmd->isOrthographic)
            {
                // map from ([0 - width], [0 - height]) -> ([-1 - 1], [-1 - 1])
                camera.transform = glm::identity<glm::mat4>();
                camera.transform = glm::scale(camera.transform, glm::vec3(2.0f / width, 2.0f / height, 1));
                camera.transform = glm::translate(camera.transform,
                    glm::vec3(-((width * 0.5f) + cmd->cameraPos.x), -((height * 0.5f) + cmd->cameraPos.y), 0));

                glDisable(GL_DEPTH_TEST);
                glDepthFunc(GL_ALWAYS);
            }
            else
            {
                float nearPlane = 0.1f;
                float farPlane = 10000;
                glm::vec3 up = glm::vec3(0, 1, 0);
                float aspectRatio = (float)width / (float)height;
                glm::mat4 projection = glm::perspective(cmd->fov, aspectRatio, nearPlane, farPlane);
                camera.transform = projection * glm::lookAt(cmd->cameraPos, cmd->lookAt, up);

                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
            }

            glBindBuffer(GL_UNIFORM_BUFFER, ctx->cameraUniformBufferId);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera), &camera);
        }
        break;
        case RENDER_CMD_SetLightingCommand:
        {
            SetLightingCommand *cmd = (SetLightingCommand *)commandData;

            OpenGlLightingState lighting;
            lighting.lightDir = cmd->lightDir;
            lighting.isEnabled = cmd->isEnabled;
            lighting.isTextureEnabled = cmd->isTextureEnabled;
            lighting.isNormalMapEnabled = cmd->isNormalMapEnabled;
            lighting.isAOMapEnabled = cmd->isAOMapEnabled;
            lighting.isDisplacementMapEnabled = cmd->isDisplacementMapEnabled;

            glBindBuffer(GL_UNIFORM_BUFFER, ctx->lightingUniformBufferId);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lighting), &lighting);
        }
        break;
        case RENDER_CMD_ClearCommand:
        {
            ClearCommand *cmd = (ClearCommand *)commandData;
            glClearColor(cmd->color.r, cmd->color.g, cmd->color.b, cmd->color.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        break;
        case RENDER_CMD_DrawQuadsCommand:
        {
            DrawQuadsCommand *cmd = (DrawQuadsCommand *)commandData;
            if (applyEffect(cmd->effect))
            {
                uint32 vertexBufferId =
                    cmd->isTopDown ? ctx->quadTopDownVertexBufferId : ctx->quadBottomUpVertexBufferId;
                uint32 vertexBufferStride = 4 * sizeof(float);
                glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->quadElementBufferId);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 2, GL_FLOAT, false, vertexBufferStride, (void *)0);
                glVertexAttribPointer(1, 2, GL_FLOAT, false, vertexBufferStride, (void *)(2 * sizeof(float)));

                uint32 instanceBufferStride = 4 * sizeof(float);
                glBindBuffer(GL_ARRAY_BUFFER, ctx->quadInstanceBufferId);
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 4, GL_FLOAT, false, instanceBufferStride, (void *)0);
                glVertexAttribDivisor(2, 1);

                glDrawElementsInstancedBaseInstance(
                    GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, cmd->instanceCount, cmd->instanceOffset);

                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                glDisableVertexAttribArray(2);
            }
            else
            {
                isMissingResources = true;
            }
        }
        break;
        case RENDER_CMD_DrawMeshesCommand:
        {
            DrawMeshesCommand *cmd = (DrawMeshesCommand *)commandData;
            if (applyEffect(cmd->effect) && cmd->mesh.ptr != 0)
            {
                OpenGlMesh *glMesh = (OpenGlMesh *)cmd->mesh.ptr;
                uint32 vertexBufferStride = 6 * sizeof(float);
                glBindBuffer(GL_ARRAY_BUFFER, glMesh->vertexBufferId);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh->elementBufferId);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 3, GL_FLOAT, false, vertexBufferStride, (void *)0);
                glVertexAttribPointer(1, 3, GL_FLOAT, false, vertexBufferStride, (void *)(3 * sizeof(float)));

                uint32 instanceBufferStride = sizeof(RenderMeshInstance);
                glBindBuffer(GL_ARRAY_BUFFER, ctx->meshInstanceBufferId);
                glEnableVertexAttribArray(2);
                glEnableVertexAttribArray(3);
                glEnableVertexAttribArray(4);
                glEnableVertexAttribArray(5);
                glEnableVertexAttribArray(6);
                glVertexAttribIPointer(
                    2, 1, GL_UNSIGNED_INT, instanceBufferStride, offsetOf(RenderMeshInstance, id));
                glVertexAttribDivisor(2, 1);
                glVertexAttribPointer(
                    3, 4, GL_FLOAT, false, instanceBufferStride, offsetOf(RenderMeshInstance, transform));
                glVertexAttribDivisor(3, 1);
                glVertexAttribPointer(
                    4, 4, GL_FLOAT, false, instanceBufferStride, offsetOf(RenderMeshInstance, transform) + 16);
                glVertexAttribDivisor(4, 1);
                glVertexAttribPointer(
                    5, 4, GL_FLOAT, false, instanceBufferStride, offsetOf(RenderMeshInstance, transform) + 32);
                glVertexAttribDivisor(5, 1);
                glVertexAttribPointer(
                    6, 4, GL_FLOAT, false, instanceBufferStride, offsetOf(RenderMeshInstance, transform) + 48);
                glVertexAttribDivisor(6, 1);

                glDrawElementsInstancedBaseInstance(GL_TRIANGLES, glMesh->elementCount, GL_UNSIGNED_INT, 0,
                    cmd->instanceCount, cmd->instanceOffset);

                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                glDisableVertexAttribArray(2);
                glDisableVertexAttribArray(3);
                glDisableVertexAttribArray(4);
                glDisableVertexAttribArray(5);
                glDisableVertexAttribArray(6);
            }
            else
            {
                isMissingResources = true;
            }
        }
        break;
        case RENDER_CMD_DrawTerrainCommand:
        {
            DrawTerrainCommand *cmd = (DrawTerrainCommand *)commandData;
            if (cmd->terrainShader.ptr != 0)
            {
                Heightfield *heightfield = cmd->heightfield;
                uint32 calcTessLevelShaderProgramId = shaders->terrainCalcTessLevelShaderProgramId;
                uint32 terrainShaderProgramId = getShaderProgramId(cmd->terrainShader);
                uint32 meshEdgeCount =
                    (2 * (heightfield->rows * heightfield->columns)) - heightfield->rows - heightfield->columns;
                glm::vec3 terrainDimensions = glm::vec3(heightfield->spacing * (heightfield->columns - 1),
                    heightfield->maxHeight, heightfield->spacing * (heightfield->rows - 1));

                // calculate tessellation levels
                glPatchParameteri(GL_PATCH_VERTICES, 4);
                glUseProgram(calcTessLevelShaderProgramId);
                glProgramUniform1f(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "targetTriangleSize"), 0.015f);
                glProgramUniform1i(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "horizontalEdgeCount"),
                    heightfield->rows * (heightfield->columns - 1));
                glProgramUniform1i(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "columnCount"), heightfield->columns);
                glProgramUniform1f(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "terrainHeight"), heightfield->maxHeight);
                glProgramUniform2fv(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "terrainOrigin"), 1,
                    glm::value_ptr(heightfield->center));
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, getTextureId(cmd->heightmapTexture));
                glActiveTexture(GL_TEXTURE6);
                glBindTexture(GL_TEXTURE_2D, getTextureId(cmd->xAdjacentHeightmapTexture));
                glActiveTexture(GL_TEXTURE7);
                glBindTexture(GL_TEXTURE_2D, getTextureId(cmd->xAdjacentReferenceHeightmapTexture));
                glActiveTexture(GL_TEXTURE8);
                glBindTexture(GL_TEXTURE_2D, getTextureId(cmd->yAdjacentHeightmapTexture));
                glActiveTexture(GL_TEXTURE9);
                glBindTexture(GL_TEXTURE_2D, getTextureId(cmd->yAdjacentReferenceHeightmapTexture));
                glActiveTexture(GL_TEXTURE10);
                glBindTexture(GL_TEXTURE_2D, getTextureId(cmd->oppositeHeightmapTexture));
                glActiveTexture(GL_TEXTURE11);
                glBindTexture(GL_TEXTURE_2D, getTextureId(cmd->oppositeReferenceHeightmapTexture));
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cmd->tessellationLevelBufferId);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cmd->meshVertexBufferId);
                glDispatchCompute(meshEdgeCount, 1, 1);
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

                // draw terrain mesh
                glUseProgram(terrainShaderProgramId);
                glPolygonMode(GL_FRONT_AND_BACK, cmd->isWireframe ? GL_LINE : GL_FILL);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D_ARRAY, cmd->albedoTextureArrayId);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D_ARRAY, cmd->normalTextureArrayId);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D_ARRAY, cmd->displacementTextureArrayId);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D_ARRAY, cmd->aoTextureArrayId);
                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_2D, getTextureId(cmd->referenceHeightmapTexture));
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cmd->materialPropsBufferId);
                glProgramUniform2fv(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "terrainOrigin"), 1,
                    glm::value_ptr(heightfield->center));
                glProgramUniform2fv(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "heightmapSize"), 1,
                    glm::value_ptr(cmd->heightmapSize));
                glProgramUniform1i(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "materialCount"), cmd->materialCount);
                glProgramUniform3fv(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "terrainDimensions"), 1,
                    glm::value_ptr(terrainDimensions));
                glProgramUniform1i(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "visualizationMode"), cmd->visualizationMode);
                glProgramUniform2fv(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "cursorPos"), 1, glm::value_ptr(cmd->cursorPos));
                glProgramUniform1f(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "cursorRadius"), cmd->cursorRadius);
                glProgramUniform1f(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "cursorFalloff"), cmd->cursorFalloff);

                uint32 vertexBufferStride = 5 * sizeof(float);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd->meshElementBufferId);
                glBindBuffer(GL_ARRAY_BUFFER, cmd->meshVertexBufferId);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 3, GL_FLOAT, false, vertexBufferStride, 0);
                glVertexAttribPointer(1, 2, GL_FLOAT, false, vertexBufferStride, (void *)(3 * sizeof(float)));

                glDrawElements(GL_PATCHES, cmd->meshElementCount, GL_UNSIGNED_INT, 0);

                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
            }
            else
            {
                isMissingResources = false;
            }
        }
        break;
        }

        command = command->next;
    }

    if (target)
    {
        glBindTexture(GL_TEXTURE_2D, getTextureId(target->textureHandle));
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    return !isMissingResources;
}