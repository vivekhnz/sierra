#include "engine_renderer.h"

#define RENDERER_CAMERA_UBO_SLOT 0
#define RENDERER_LIGHTING_UBO_SLOT 1

extern EnginePlatformApi Platform;
global_variable bool WasRendererReloaded = true;

ASSETS_GET_SHADER(assetsGetShader);
ASSETS_GET_MESH(assetsGetMesh);

struct RendererInternalShaders
{
    bool initialized;

    uint32 quadVertexShaderId;
    uint32 quadFragmentShaderId;
    uint32 texturedQuadShaderProgramId;

    uint32 meshVertexShaderId;

    uint32 terrainVertexShaderId;
    uint32 terrainTessCtrlShaderId;
    uint32 terrainTessEvalShaderId;
    uint32 terrainCalcTessLevelShaderId;
    uint32 terrainCalcTessLevelShaderProgramId;
};

struct RenderContext
{
    // access this via getInternalShaders which handles the engine DLL being reloaded
    RendererInternalShaders _shaders;
    uint32 globalVertexArrayId;

    uint32 quadElementBufferId;
    uint32 quadTopDownVertexBufferId;
    uint32 quadBottomUpVertexBufferId;

    uint32 quadInstanceBufferId;
    RenderQuad *quads;
    uint32 maxQuads;

    uint32 meshInstanceBufferId;
    RenderMeshInstance *meshInstances;
    uint32 maxMeshInstances;

    uint32 cameraUniformBufferId;
    uint32 lightingUniformBufferId;
};

struct GpuCameraState
{
    glm::mat4 transform;
};
struct GpuLightingState
{
    glm::vec4 lightDir;

    // todo: pack these into a single uint8
    uint32 isEnabled;
    uint32 isTextureEnabled;
    uint32 isNormalMapEnabled;
    uint32 isAOMapEnabled;
    uint32 isDisplacementMapEnabled;
};

enum RenderEffectParameterType
{
    EFFECT_PARAM_TYPE_FLOAT,
    EFFECT_PARAM_TYPE_INT,
    EFFECT_PARAM_TYPE_UINT
};
struct RenderEffectParameter
{
    char *name;
    RenderEffectParameterType type;
    union
    {
        float f;
        int32 i;
        uint32 u;
    } value;
    RenderEffectParameter *next;
};
struct RenderEffectTexture
{
    uint32 slot;
    uint32 textureId;
    RenderEffectTexture *next;
};
struct RenderEffect
{
    MemoryArena *arena;
    AssetHandle shaderHandle;
    uint32 shaderProgramId;
    RenderEffectBlendMode blendMode;
    RenderEffectParameter *firstParameter;
    RenderEffectParameter *lastParameter;
    RenderEffectTexture *firstTexture;
    RenderEffectTexture *lastTexture;
};

enum RenderQueueCommandType
{
    RENDER_CMD_SetCameraCommand,
    RENDER_CMD_ClearCommand,
    RENDER_CMD_DrawQuadsCommand,
    RENDER_CMD_DrawMeshesCommand,
    RENDER_CMD_DrawTerrainCommand
};
struct RenderQueueCommandHeader
{
    RenderQueueCommandType type;
    RenderQueueCommandHeader *next;
};

struct SetCameraCommand
{
    bool isOrthographic;

    glm::vec3 cameraPos;
    glm::vec3 lookAt;
    float fov;
};
struct ClearCommand
{
    glm::vec4 color;
};
struct DrawQuadsCommand
{
    RenderEffect *effect;
    bool isTopDown;
    uint32 instanceOffset;
    uint32 instanceCount;
};
struct DrawMeshesCommand
{
    RenderEffect *effect;
    AssetHandle mesh;
    uint32 instanceOffset;
    uint32 instanceCount;
};
struct DrawTerrainCommand
{
    Heightfield *heightfield;

    AssetHandle terrainShader;

    uint32 heightmapTextureId;
    uint32 referenceHeightmapTextureId;

    uint32 meshVertexBufferId;
    uint32 meshElementBufferId;
    uint32 tessellationLevelBufferId;
    uint32 meshElementCount;

    uint32 materialCount;
    uint32 albedoTextureArrayId;
    uint32 normalTextureArrayId;
    uint32 displacementTextureArrayId;
    uint32 aoTextureArrayId;
    uint32 materialPropsBufferId;

    bool isWireframe;

    uint32 visualizationMode;
    glm::vec2 cursorPos;
    float cursorRadius;
    float cursorFalloff;
};
struct RenderQueue
{
    MemoryArena *arena;
    RenderContext *ctx;

    RenderQueueCommandHeader *firstCommand;
    RenderQueueCommandHeader *lastCommand;

    uint32 quadCount;
    uint32 meshInstanceCount;
};

struct RenderTargetDescriptor
{
    uint32 elementType;
    uint32 elementSize;
    uint32 cpuFormat;
    uint32 gpuFormat;
    bool isInteger;
    bool hasDepthBuffer;
};

bool createShader(uint32 type, char *src, uint32 *out_id)
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
bool createShaderProgram(uint32 shaderCount, uint32 *shaderIds, uint32 *out_id)
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

RendererInternalShaders *getInternalShaders(RenderContext *ctx)
{
    RendererInternalShaders *shaders = &ctx->_shaders;
    if (WasRendererReloaded)
    {
        if (shaders->initialized)
        {
            glDeleteProgram(shaders->texturedQuadShaderProgramId);
            glDeleteProgram(shaders->terrainCalcTessLevelShaderProgramId);
            glDeleteShader(shaders->quadVertexShaderId);
            glDeleteShader(shaders->quadFragmentShaderId);
            glDeleteShader(shaders->meshVertexShaderId);
            glDeleteShader(shaders->terrainVertexShaderId);
            glDeleteShader(shaders->terrainTessCtrlShaderId);
            glDeleteShader(shaders->terrainTessEvalShaderId);
            glDeleteShader(shaders->terrainCalcTessLevelShaderId);
        }

        // create quad shader program
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

        char *quadFragmentShaderSrc = R"(
#version 430 core
layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D imageTexture;

out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(imageTexture, uv).rgb, 1);
}
    )";

        assert(createShader(GL_VERTEX_SHADER, quadVertexShaderSrc, &shaders->quadVertexShaderId));
        assert(createShader(GL_FRAGMENT_SHADER, quadFragmentShaderSrc, &shaders->quadFragmentShaderId));
        uint32 shaderIds[] = {shaders->quadVertexShaderId, shaders->quadFragmentShaderId};
        assert(createShaderProgram(2, shaderIds, &shaders->texturedQuadShaderProgramId));

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
        assert(createShader(GL_VERTEX_SHADER, meshVertexShaderSrc, &shaders->meshVertexShaderId));

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
            gl_TessLevelOuter[0] = abs(A.y); // AB
            gl_TessLevelOuter[1] = abs(A.x); // AD
            gl_TessLevelOuter[2] = abs(C.w); // CD
            gl_TessLevelOuter[3] = abs(C.z); // BC
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

layout(binding = 0) uniform sampler2D activeHeightmapTexture;
layout(binding = 3) uniform sampler2DArray displacementTextures;
layout(binding = 5) uniform sampler2D referenceHeightmapTexture;

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
float height(vec2 uv)
{
    return textureLod(activeHeightmapTexture, uv, 2).x;
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
    float normalSampleOffsetInTexels = 2;
    vec2 normalSampleOffsetInUvCoords = normalSampleOffsetInTexels / vec2(2048, 2048);
    float hL = height(vec2(hUV.x - normalSampleOffsetInUvCoords.x, hUV.y));
    float hR = height(vec2(hUV.x + normalSampleOffsetInUvCoords.x, hUV.y));
    float hD = height(vec2(hUV.x, hUV.y - normalSampleOffsetInUvCoords.y));
    float hU = height(vec2(hUV.x, hUV.y + normalSampleOffsetInUvCoords.y));
    
    float nY = (2 * terrainDimensions.x * normalSampleOffsetInUvCoords.x) / terrainDimensions.y;
    normal = normalize(vec3(hL - hR, nY, hU - hD));
    float slope = 1 - normal.y;
    
    // calculate texture coordinates
    vec3 triBlend = calcTriplanarBlend(normal);
    vec3 triAxisSign = sign(normal);
    texcoord = vec3(
        hUV.x * terrainDimensions.x,
        -altitude * terrainDimensions.y,
        hUV.y * terrainDimensions.z);

    vec2 baseTexcoordsX = vec2(texcoord.z * triAxisSign.x, texcoord.y);
    vec2 baseTexcoordsY = vec2(texcoord.x * triAxisSign.y, texcoord.z);
    vec2 baseTexcoordsZ = vec2(texcoord.x * triAxisSign.z, texcoord.y);
    
    vec3 pos = lerp3D(in_worldPos[0], in_worldPos[1], in_worldPos[2], in_worldPos[3]);
    pos.y = altitude * terrainDimensions.y;

    heights.x = textureLod(referenceHeightmapTexture, hUV, 2).x;
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
layout(binding = 0) uniform sampler2D heightmapTexture;

vec3 worldToScreen(vec3 p)
{
    vec4 clipPos = camera_transform * vec4(p, 1.0f);
    return clipPos.xyz / clipPos.w;
}

float height(vec2 uv)
{
    return texture(heightmapTexture, uv).x * terrainHeight;
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

        assert(createShader(GL_VERTEX_SHADER, terrainVertexShaderSrc, &shaders->terrainVertexShaderId));
        assert(createShader(GL_TESS_CONTROL_SHADER, terrainTessCtrlShaderSrc, &shaders->terrainTessCtrlShaderId));
        assert(
            createShader(GL_TESS_EVALUATION_SHADER, terrainTessEvalShaderSrc, &shaders->terrainTessEvalShaderId));
        assert(createShader(
            GL_COMPUTE_SHADER, terrainCalcTessLevelShaderSrc, &shaders->terrainCalcTessLevelShaderId));
        assert(createShaderProgram(
            1, &shaders->terrainCalcTessLevelShaderId, &shaders->terrainCalcTessLevelShaderProgramId));

        shaders->initialized = true;
        WasRendererReloaded = false;
    }
    return shaders;
}

bool createShaderProgram(RenderContext *rctx, ShaderType type, char *src, uint32 *out_programId)
{
    bool result = false;
    RendererInternalShaders *shaders = getInternalShaders(rctx);

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

    if (createShader(GL_FRAGMENT_SHADER, src, &shaderIds[shaderCount - 1]))
    {
        uint32 programId;
        if (createShaderProgram(shaderCount, shaderIds, &programId))
        {
            *out_programId = programId;
            result = true;
        }
    }

    return result;
}
void destroyShaderProgram(uint32 id)
{
    glDeleteProgram(id);
}

RenderMesh *createMesh(MemoryArena *arena, void *vertices, uint32 vertexCount, void *indices, uint32 indexCount)
{
    RenderMesh *result = pushStruct(arena, RenderMesh);

    uint32 vertexBufferStride = 6 * sizeof(float);
    glGenBuffers(1, &result->vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, result->vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexBufferStride, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &result->elementBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result->elementBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(uint32), indices, GL_STATIC_DRAW);

    return result;
}
void destroyMesh(RenderMesh *mesh)
{
    glDeleteBuffers(1, &mesh->vertexBufferId);
    glDeleteBuffers(1, &mesh->elementBufferId);
}

RENDERER_INITIALIZE(rendererInitialize)
{
    RenderContext *ctx = pushStruct(arena, RenderContext);

    // setup global state
    glGenVertexArrays(1, &ctx->globalVertexArrayId);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    // initialize camera state
    glGenBuffers(1, &ctx->cameraUniformBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, ctx->cameraUniformBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuCameraState), 0, GL_DYNAMIC_DRAW);
    glBindBufferRange(
        GL_UNIFORM_BUFFER, RENDERER_CAMERA_UBO_SLOT, ctx->cameraUniformBufferId, 0, sizeof(GpuCameraState));

    // initialize lighting state
    GpuLightingState lighting;
    lighting.lightDir = glm::vec4(-0.588f, 0.809f, 0.294f, 0.0f);
    lighting.isEnabled = true;
    lighting.isTextureEnabled = true;
    lighting.isNormalMapEnabled = true;
    lighting.isAOMapEnabled = true;
    lighting.isDisplacementMapEnabled = true;

    glGenBuffers(1, &ctx->lightingUniformBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, ctx->lightingUniformBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(lighting), &lighting, GL_DYNAMIC_DRAW);
    glBindBufferRange(
        GL_UNIFORM_BUFFER, RENDERER_LIGHTING_UBO_SLOT, ctx->lightingUniformBufferId, 0, sizeof(lighting));

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
    ctx->maxQuads = 65536;
    ctx->quads = (RenderQuad *)pushSize(arena, sizeof(RenderQuad) * ctx->maxQuads);

    glGenBuffers(1, &ctx->meshInstanceBufferId);
    ctx->maxMeshInstances = 4096;
    ctx->meshInstances = (RenderMeshInstance *)pushSize(arena, sizeof(RenderMeshInstance) * ctx->maxMeshInstances);

    return ctx;
}

RENDERER_UPDATE_LIGHTING_STATE(rendererUpdateLightingState)
{
    GpuLightingState lighting;
    lighting.lightDir = *lightDir;
    lighting.isEnabled = isLightingEnabled;
    lighting.isTextureEnabled = isTextureEnabled;
    lighting.isNormalMapEnabled = isNormalMapEnabled;
    lighting.isAOMapEnabled = isAOMapEnabled;
    lighting.isDisplacementMapEnabled = isDisplacementMapEnabled;

    glBindBuffer(GL_UNIFORM_BUFFER, ctx->lightingUniformBufferId);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lighting), &lighting);
}

RENDERER_CREATE_TEXTURE(rendererCreateTexture)
{
    uint32 id = 0;
    glGenTextures(1, &id);

    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexImage2D(GL_TEXTURE_2D, 0, cpuFormat, width, height, 0, gpuFormat, elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    return id;
}

RENDERER_UPDATE_TEXTURE(rendererUpdateTexture)
{
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, cpuFormat, width, height, 0, gpuFormat, elementType, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
}

RENDERER_READ_TEXTURE_PIXELS(rendererReadTexturePixels)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, gpuFormat, elementType, out_pixels);
}

RENDERER_CREATE_TEXTURE_ARRAY(rendererCreateTextureArray)
{
    uint32 id = 0;
    glGenTextures(1, &id);

    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, cpuFormat, width, height, layers, 0, gpuFormat, elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    return id;
}

RENDERER_UPDATE_TEXTURE_ARRAY(rendererUpdateTextureArray)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, width, height, 1, gpuFormat, elementType, pixels);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

RENDERER_CREATE_BUFFER(rendererCreateBuffer)
{
    RenderBuffer result = {};
    result.usage = usage;

    glGenBuffers(1, &result.id);

    if (type == RENDERER_VERTEX_BUFFER)
    {
        result.type = GL_ARRAY_BUFFER;
    }
    else if (type == RENDERER_ELEMENT_BUFFER)
    {
        result.type = GL_ELEMENT_ARRAY_BUFFER;
    }
    else if (type == RENDERER_SHADER_STORAGE_BUFFER)
    {
        result.type = GL_SHADER_STORAGE_BUFFER;
    }
    assert(result.type);

    return result;
}

RENDERER_UPDATE_BUFFER(rendererUpdateBuffer)
{
    glBindBuffer(buffer->type, buffer->id);
    glBufferData(buffer->type, size, data, buffer->usage);
}

// render targets

RenderTargetDescriptor getRenderTargetDescriptor(RenderTargetFormat format)
{
    RenderTargetDescriptor result = {};
    if (format == RENDER_TARGET_FORMAT_RGB8_WITH_DEPTH)
    {
        result.elementType = GL_UNSIGNED_BYTE;
        result.elementSize = sizeof(uint8);
        result.cpuFormat = GL_RGB;
        result.gpuFormat = GL_RGB;
        result.isInteger = false;
        result.hasDepthBuffer = true;
    }
    else if (format == RENDER_TARGET_FORMAT_R16)
    {
        result.elementType = GL_UNSIGNED_SHORT;
        result.elementSize = sizeof(uint16);
        result.cpuFormat = GL_R16;
        result.gpuFormat = GL_RED;
        result.isInteger = false;
        result.hasDepthBuffer = false;
    }
    else if (format == RENDER_TARGET_FORMAT_R8UI_WITH_DEPTH)
    {
        result.elementType = GL_UNSIGNED_BYTE;
        result.elementSize = sizeof(uint8);
        result.cpuFormat = GL_R8UI;
        result.gpuFormat = GL_RED_INTEGER;
        result.isInteger = true;
        result.hasDepthBuffer = true;
    }
    else if (format == RENDER_TARGET_FORMAT_R16UI_WITH_DEPTH)
    {
        result.elementType = GL_UNSIGNED_SHORT;
        result.elementSize = sizeof(uint16);
        result.cpuFormat = GL_R16UI;
        result.gpuFormat = GL_RED_INTEGER;
        result.isInteger = true;
        result.hasDepthBuffer = true;
    }
    else if (format == RENDER_TARGET_FORMAT_R32UI_WITH_DEPTH)
    {
        result.elementType = GL_UNSIGNED_INT;
        result.elementSize = sizeof(uint32);
        result.cpuFormat = GL_R32UI;
        result.gpuFormat = GL_RED_INTEGER;
        result.isInteger = true;
        result.hasDepthBuffer = true;
    }
    else
    {
        assert(!"Unknown render target format");
    }

    return result;
}

RENDERER_CREATE_RENDER_TARGET(rendererCreateRenderTarget)
{
    RenderTarget *result = pushStruct(arena, RenderTarget);
    *result = {};
    result->format = format;
    result->width = width;
    result->height = height;

    RenderTargetDescriptor descriptor = getRenderTargetDescriptor(result->format);

    // create target texture
    glGenTextures(1, &result->textureId);
    glBindTexture(GL_TEXTURE_2D, result->textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, descriptor.isInteger ? GL_NEAREST : GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, descriptor.isInteger ? GL_NEAREST : GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D, 0, descriptor.cpuFormat, width, height, 0, descriptor.gpuFormat, descriptor.elementType, 0);
    if (!descriptor.isInteger)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // create depth buffer
    if (descriptor.hasDepthBuffer)
    {
        glGenTextures(1, &result->depthTextureId);
        glBindTexture(GL_TEXTURE_2D, result->depthTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }

    // create framebuffer
    glGenFramebuffers(1, &result->framebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, result->framebufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result->textureId, 0);
    if (descriptor.hasDepthBuffer)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, result->depthTextureId, 0);
    }
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return result;
}

RENDERER_RESIZE_RENDER_TARGET(rendererResizeRenderTarget)
{
    target->width = width;
    target->height = height;

    RenderTargetDescriptor descriptor = getRenderTargetDescriptor(target->format);

    glBindTexture(GL_TEXTURE_2D, target->textureId);
    glTexImage2D(
        GL_TEXTURE_2D, 0, descriptor.cpuFormat, width, height, 0, descriptor.gpuFormat, descriptor.elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (descriptor.hasDepthBuffer)
    {
        glBindTexture(GL_TEXTURE_2D, target->depthTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }
}

RENDERER_GET_PIXELS(rendererGetPixels)
{
    RenderTargetDescriptor descriptor = getRenderTargetDescriptor(target->format);

    uint32 pixelCount = width * height;
    uint32 bufferSize = pixelCount * descriptor.elementSize;
    void *buffer = pushSize(arena, bufferSize);

    glGetTextureSubImage(target->textureId, 0, x, y, 0, width, height, 1, descriptor.gpuFormat,
        descriptor.elementType, bufferSize, buffer);

    *out_pixelCount = pixelCount;
    return buffer;
}

// effects

RENDERER_CREATE_EFFECT(rendererCreateEffect)
{
    RenderEffect *result = pushStruct(arena, RenderEffect);
    *result = {};

    result->arena = arena;
    result->shaderHandle = shader;
    result->shaderProgramId = 0;
    result->blendMode = blendMode;
    result->firstParameter = 0;
    result->lastParameter = 0;

    return result;
}

inline RenderEffectParameter *pushEffectParameter(RenderEffect *effect, char *paramName)
{
    RenderEffectParameter *param = pushStruct(effect->arena, RenderEffectParameter);
    *param = {};
    param->next = 0;

    char *srcCursor = paramName;
    uint32 length = 0;
    while (*srcCursor++)
    {
        length++;
    }
    param->name = (char *)pushSize(effect->arena, length + 1);
    char *dstCursor = param->name;
    srcCursor = paramName;
    while (*srcCursor)
    {
        *dstCursor++ = *srcCursor++;
    }
    *dstCursor = 0;

    if (effect->lastParameter)
    {
        effect->lastParameter->next = param;
    }
    effect->lastParameter = param;
    if (!effect->firstParameter)
    {
        effect->firstParameter = param;
    }

    return param;
}

RENDERER_SET_EFFECT_FLOAT(rendererSetEffectFloat)
{
    RenderEffectParameter *param = pushEffectParameter(effect, paramName);
    param->type = EFFECT_PARAM_TYPE_FLOAT;
    param->value.f = value;
}

RENDERER_SET_EFFECT_INT(rendererSetEffectInt)
{
    RenderEffectParameter *param = pushEffectParameter(effect, paramName);
    param->type = EFFECT_PARAM_TYPE_INT;
    param->value.i = value;
}

RENDERER_SET_EFFECT_UINT(rendererSetEffectUint)
{
    RenderEffectParameter *param = pushEffectParameter(effect, paramName);
    param->type = EFFECT_PARAM_TYPE_UINT;
    param->value.u = value;
}

RENDERER_SET_EFFECT_TEXTURE(rendererSetEffectTexture)
{
    RenderEffectTexture *texture = pushStruct(effect->arena, RenderEffectTexture);
    *texture = {};
    texture->slot = slot;
    texture->textureId = textureId;
    texture->next = 0;

    if (effect->lastTexture)
    {
        effect->lastTexture->next = texture;
    }
    effect->lastTexture = texture;
    if (!effect->firstTexture)
    {
        effect->firstTexture = texture;
    }
}

// render queue

RENDERER_CREATE_QUEUE(rendererCreateQueue)
{
    RenderQueue *result = pushStruct(arena, RenderQueue);
    *result = {};
    result->arena = arena;
    result->ctx = ctx;

    return result;
}

void *pushRenderCommandInternal(RenderQueue *rq, RenderQueueCommandType type, uint64 size)
{
    RenderQueueCommandHeader *header = pushStruct(rq->arena, RenderQueueCommandHeader);
    *header = {};
    header->type = type;

    if (rq->lastCommand)
    {
        rq->lastCommand->next = header;
    }
    rq->lastCommand = header;
    if (!rq->firstCommand)
    {
        rq->firstCommand = header;
    }

    void *commandData = pushSize(rq->arena, size);
    memset(commandData, 0, size);
    return commandData;
}
#define pushRenderCommand(rq, type) (type *)pushRenderCommandInternal(rq, RENDER_CMD_##type, sizeof(type))

RENDERER_SET_CAMERA_ORTHO(rendererSetCameraOrtho)
{
    SetCameraCommand *cmd = pushRenderCommand(rq, SetCameraCommand);
    cmd->isOrthographic = true;
}
RENDERER_SET_CAMERA_PERSP(rendererSetCameraPersp)
{
    SetCameraCommand *cmd = pushRenderCommand(rq, SetCameraCommand);
    cmd->isOrthographic = false;
    cmd->cameraPos = cameraPos;
    cmd->lookAt = lookAt;
    cmd->fov = fov;
}

RENDERER_CLEAR(rendererClear)
{
    ClearCommand *cmd = pushRenderCommand(rq, ClearCommand);
    cmd->color.r = r;
    cmd->color.g = g;
    cmd->color.b = b;
    cmd->color.a = a;
}

void pushQuads(RenderQueue *rq, RenderQuad *quads, uint32 quadCount, RenderEffect *effect, bool isTopDown)
{
    assert(rq->quadCount + quadCount < rq->ctx->maxQuads);

    DrawQuadsCommand *cmd = pushRenderCommand(rq, DrawQuadsCommand);
    cmd->effect = effect;
    cmd->isTopDown = isTopDown;
    cmd->instanceOffset = rq->quadCount;
    cmd->instanceCount = quadCount;

    memcpy(rq->ctx->quads + rq->quadCount, quads, sizeof(RenderQuad) * quadCount);
    rq->quadCount += quadCount;
}

RENDERER_PUSH_TEXTURED_QUAD(rendererPushTexturedQuad)
{
    RendererInternalShaders *shaders = getInternalShaders(rq->ctx);

    RenderEffect *effect = rendererCreateEffect(rq->arena, 0, EFFECT_BLEND_ALPHA_BLEND);
    effect->shaderProgramId = shaders->texturedQuadShaderProgramId;

    rendererSetEffectTexture(effect, 0, textureId);
    pushQuads(rq, &quad, 1, effect, isTopDown);
}

RENDERER_PUSH_QUAD(rendererPushQuad)
{
    pushQuads(rq, &quad, 1, effect, true);
}
RENDERER_PUSH_QUADS(rendererPushQuads)
{
    pushQuads(rq, quads, quadCount, effect, true);
}

RENDERER_PUSH_MESHES(rendererPushMeshes)
{
    assert(rq->meshInstanceCount + instanceCount < rq->ctx->maxMeshInstances);

    DrawMeshesCommand *cmd = pushRenderCommand(rq, DrawMeshesCommand);
    cmd->effect = effect;
    cmd->mesh = mesh;
    cmd->instanceOffset = rq->meshInstanceCount;
    cmd->instanceCount = instanceCount;

    memcpy(rq->ctx->meshInstances + rq->meshInstanceCount, instances, sizeof(RenderMeshInstance) * instanceCount);
    rq->meshInstanceCount += instanceCount;
}

RENDERER_PUSH_TERRAIN(rendererPushTerrain)
{
    DrawTerrainCommand *cmd = pushRenderCommand(rq, DrawTerrainCommand);

    cmd->heightfield = heightfield;

    cmd->terrainShader = terrainShader;

    cmd->heightmapTextureId = heightmapTextureId;
    cmd->referenceHeightmapTextureId = referenceHeightmapTextureId;

    cmd->meshVertexBufferId = meshVertexBufferId;
    cmd->meshElementBufferId = meshElementBufferId;
    cmd->tessellationLevelBufferId = tessellationLevelBufferId;
    cmd->meshElementCount = meshElementCount;

    cmd->materialCount = materialCount;
    cmd->albedoTextureArrayId = albedoTextureArrayId;
    cmd->normalTextureArrayId = normalTextureArrayId;
    cmd->displacementTextureArrayId = displacementTextureArrayId;
    cmd->aoTextureArrayId = aoTextureArrayId;
    cmd->materialPropsBufferId = materialPropsBufferId;

    cmd->isWireframe = isWireframe;

    cmd->visualizationMode = visualizationMode;
    cmd->cursorPos = cursorPos;
    cmd->cursorRadius = cursorRadius;
    cmd->cursorFalloff = cursorFalloff;
}

bool applyEffect(RenderEffect *effect)
{
    bool isMissingResources = false;

    // load shader from asset system via an asset handle if one was specified
    // otherwise, we assume that the shader program ID was explicitly set on the effect
    uint32 shaderProgramId;
    if (effect->shaderHandle)
    {
        LoadedAsset *shader = assetsGetShader(effect->shaderHandle);
        if (shader->shader)
        {
            shaderProgramId = shader->shader->id;
        }
        else
        {
            isMissingResources = true;
        }
    }
    else
    {
        shaderProgramId = effect->shaderProgramId;
    }

    if (!isMissingResources)
    {
        glUseProgram(shaderProgramId);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);

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
            glBindTexture(GL_TEXTURE_2D, effectTexture->textureId);

            effectTexture = effectTexture->next;
        }
    }

    return !isMissingResources;
}

bool drawToTarget(RenderQueue *rq, uint32 width, uint32 height, RenderTarget *target)
{
    if (target)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, target->framebufferId);
    }
    glViewport(0, 0, width, height);

    glBindBuffer(GL_ARRAY_BUFFER, rq->ctx->quadInstanceBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RenderQuad) * rq->quadCount, rq->ctx->quads, GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, rq->ctx->meshInstanceBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RenderMeshInstance) * rq->meshInstanceCount, rq->ctx->meshInstances,
        GL_STREAM_DRAW);

    glBindVertexArray(rq->ctx->globalVertexArrayId);

    RendererInternalShaders *shaders = getInternalShaders(rq->ctx);

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

            GpuCameraState camera = {};
            if (cmd->isOrthographic)
            {
                camera.transform = glm::identity<glm::mat4>();
                camera.transform = glm::scale(camera.transform, glm::vec3(2, 2, 1));
                camera.transform = glm::translate(camera.transform, glm::vec3(-0.5f, -0.5f, 0));
            }
            else
            {
                float nearPlane = 0.1f;
                float farPlane = 10000;
                glm::vec3 up = glm::vec3(0, 1, 0);
                float aspectRatio = (float)width / (float)height;
                glm::mat4 projection = glm::perspective(cmd->fov, aspectRatio, nearPlane, farPlane);
                camera.transform = projection * glm::lookAt(cmd->cameraPos, cmd->lookAt, up);
            }

            glBindBuffer(GL_UNIFORM_BUFFER, rq->ctx->cameraUniformBufferId);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera), &camera);
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
                    cmd->isTopDown ? rq->ctx->quadTopDownVertexBufferId : rq->ctx->quadBottomUpVertexBufferId;
                uint32 vertexBufferStride = 4 * sizeof(float);
                glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rq->ctx->quadElementBufferId);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 2, GL_FLOAT, false, vertexBufferStride, (void *)0);
                glVertexAttribPointer(1, 2, GL_FLOAT, false, vertexBufferStride, (void *)(2 * sizeof(float)));

                uint32 instanceBufferStride = 4 * sizeof(float);
                glBindBuffer(GL_ARRAY_BUFFER, rq->ctx->quadInstanceBufferId);
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
            LoadedAsset *meshAsset = assetsGetMesh(cmd->mesh);
            MeshAsset *mesh = meshAsset->mesh;
            if (applyEffect(cmd->effect) && mesh)
            {
                uint32 vertexBufferStride = 6 * sizeof(float);
                glBindBuffer(GL_ARRAY_BUFFER, mesh->renderMesh->vertexBufferId);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->renderMesh->elementBufferId);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 3, GL_FLOAT, false, vertexBufferStride, (void *)0);
                glVertexAttribPointer(1, 3, GL_FLOAT, false, vertexBufferStride, (void *)(3 * sizeof(float)));

                uint32 instanceBufferStride = sizeof(RenderMeshInstance);
                glBindBuffer(GL_ARRAY_BUFFER, rq->ctx->meshInstanceBufferId);
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

                glDrawElementsInstancedBaseInstance(
                    GL_TRIANGLES, mesh->elementCount, GL_UNSIGNED_INT, 0, cmd->instanceCount, cmd->instanceOffset);

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

            LoadedAsset *terrainShader = assetsGetShader(cmd->terrainShader);
            if (terrainShader->shader)
            {
                Heightfield *heightfield = cmd->heightfield;
                uint32 calcTessLevelShaderProgramId = shaders->terrainCalcTessLevelShaderProgramId;
                uint32 terrainShaderProgramId = terrainShader->shader->id;
                uint32 meshEdgeCount =
                    (2 * (heightfield->rows * heightfield->columns)) - heightfield->rows - heightfield->columns;
                glm::vec3 terrainDimensions = glm::vec3(heightfield->spacing * heightfield->columns,
                    heightfield->maxHeight, heightfield->spacing * heightfield->rows);

                // calculate tessellation levels
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
                glBindTexture(GL_TEXTURE_2D, cmd->heightmapTextureId);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cmd->tessellationLevelBufferId);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cmd->meshVertexBufferId);
                glDispatchCompute(meshEdgeCount, 1, 1);
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

                // draw terrain mesh
                glUseProgram(terrainShaderProgramId);
                glPolygonMode(GL_FRONT_AND_BACK, cmd->isWireframe ? GL_LINE : GL_FILL);
                glEnable(GL_DEPTH_TEST);
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
                glBindTexture(GL_TEXTURE_2D, cmd->referenceHeightmapTextureId);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cmd->materialPropsBufferId);
                glProgramUniform2fv(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "terrainOrigin"), 1,
                    glm::value_ptr(heightfield->center));
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
        glBindTexture(GL_TEXTURE_2D, target->textureId);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    return !isMissingResources;
}

RENDERER_DRAW_TO_TARGET(rendererDrawToTarget)
{
    return drawToTarget(rq, target->width, target->height, target);
}
RENDERER_DRAW_TO_SCREEN(rendererDrawToScreen)
{
    return drawToTarget(rq, width, height, 0);
}