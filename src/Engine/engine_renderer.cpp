#include "engine_renderer.h"

#define RENDERER_CAMERA_UBO_SLOT 0
#define RENDERER_LIGHTING_UBO_SLOT 1

ASSETS_GET_SHADER(assetsGetShader);
ASSETS_GET_MESH(assetsGetMesh);

struct RenderContext
{
    RenderBackendContext internalCtx;

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

enum RenderEffectParameterType
{
    EFFECT_PARAM_TYPE_FLOAT,
    EFFECT_PARAM_TYPE_VEC3,
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
        glm::vec3 v3;
    } value;
    RenderEffectParameter *next;
};
struct RenderEffectTexture
{
    uint32 slot;
    TextureHandle handle;
    RenderEffectTexture *next;
};
struct RenderEffect
{
    MemoryArena *arena;
    ShaderHandle shaderHandle;
    RenderEffectBlendMode blendMode;
    RenderEffectParameter *firstParameter;
    RenderEffectParameter *lastParameter;
    RenderEffectTexture *firstTexture;
    RenderEffectTexture *lastTexture;
};

enum RenderQueueCommandType
{
    RENDER_CMD_SetCameraCommand,
    RENDER_CMD_SetLightingCommand,
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
struct SetLightingCommand
{
    glm::vec4 lightDir;

    uint32 isEnabled;
    uint32 isTextureEnabled;
    uint32 isNormalMapEnabled;
    uint32 isAOMapEnabled;
    uint32 isDisplacementMapEnabled;
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
    MeshHandle mesh;
    uint32 instanceOffset;
    uint32 instanceCount;
};
struct DrawTerrainCommand
{
    Heightfield *heightfield;
    glm::vec2 heightmapSize;

    ShaderHandle terrainShader;

    TextureHandle heightmapTexture;
    TextureHandle referenceHeightmapTexture;
    TextureHandle xAdjacentHeightmapTexture;
    TextureHandle xAdjacentReferenceHeightmapTexture;
    TextureHandle yAdjacentHeightmapTexture;
    TextureHandle yAdjacentReferenceHeightmapTexture;
    TextureHandle oppositeHeightmapTexture;
    TextureHandle oppositeReferenceHeightmapTexture;

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

RENDERER_INITIALIZE(rendererInitialize)
{
    RenderContext *ctx = pushStruct(arena, RenderContext);
    ctx->internalCtx = initializeRenderBackend(arena);

    // setup global state
    glGenVertexArrays(1, &ctx->globalVertexArrayId);

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
    glGenBuffers(1, &ctx->lightingUniformBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, ctx->lightingUniformBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuLightingState), 0, GL_DYNAMIC_DRAW);
    glBindBufferRange(
        GL_UNIFORM_BUFFER, RENDERER_LIGHTING_UBO_SLOT, ctx->lightingUniformBufferId, 0, sizeof(GpuLightingState));

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

RENDERER_CREATE_TEXTURE(rendererCreateTexture)
{
    return createTexture(width, height, format);
}
RENDERER_UPDATE_TEXTURE(rendererUpdateTexture)
{
    updateTexture(handle, width, height, pixels);
}
RENDERER_GET_PIXELS(rendererGetPixels)
{
    return getPixels(arena, handle, width, height, out_pixelCount);
}
RENDERER_GET_PIXELS_IN_REGION(rendererGetPixelsInRegion)
{
    return getPixelsInRegion(arena, handle, x, y, width, height, out_pixelCount);
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

RENDERER_CREATE_RENDER_TARGET(rendererCreateRenderTarget)
{
    return createRenderTarget(arena, width, height, format, createDepthBuffer);
}
RENDERER_RESIZE_RENDER_TARGET(rendererResizeRenderTarget)
{
    target->width = width;
    target->height = height;

    resizeRenderTarget(target, width, height);
}

// effects

RenderEffect *createEffect(MemoryArena *arena, ShaderHandle shaderHandle, RenderEffectBlendMode blendMode)
{
    RenderEffect *result = pushStruct(arena, RenderEffect);
    *result = {};

    result->arena = arena;
    result->shaderHandle = shaderHandle;
    result->blendMode = blendMode;
    result->firstParameter = 0;
    result->lastParameter = 0;

    return result;
}

RENDERER_CREATE_EFFECT(rendererCreateEffect)
{
    ShaderHandle shaderHandle = {0};
    if (shaderAsset)
    {
        LoadedAsset *shader = assetsGetShader(shaderAsset);
        if (shader->shader)
        {
            shaderHandle = shader->shader->handle;
        }
    }

    RenderEffect *result = createEffect(arena, shaderHandle, blendMode);
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

RENDERER_SET_EFFECT_VEC3(rendererSetEffectVec3)
{
    RenderEffectParameter *param = pushEffectParameter(effect, paramName);
    param->type = EFFECT_PARAM_TYPE_VEC3;
    param->value.v3 = value;
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
    texture->handle = handle;
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
    cmd->cameraPos = glm::vec3(0, 0, 0);

    return cmd;
}
RENDERER_SET_CAMERA_PERSP(rendererSetCameraPersp)
{
    SetCameraCommand *cmd = pushRenderCommand(rq, SetCameraCommand);
    cmd->isOrthographic = false;
    cmd->cameraPos = cameraPos;
    cmd->lookAt = lookAt;
    cmd->fov = fov;

    return cmd;
}

RENDERER_SET_LIGHTING(rendererSetLighting)
{
    SetLightingCommand *cmd = pushRenderCommand(rq, SetLightingCommand);
    cmd->lightDir = *lightDir;
    cmd->isEnabled = isLightingEnabled;
    cmd->isTextureEnabled = isTextureEnabled;
    cmd->isNormalMapEnabled = isNormalMapEnabled;
    cmd->isAOMapEnabled = isAOMapEnabled;
    cmd->isDisplacementMapEnabled = isDisplacementMapEnabled;

    return cmd;
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
    if (quadCount > 0)
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
}

RENDERER_PUSH_TEXTURED_QUAD(rendererPushTexturedQuad)
{
    RenderEffect *effect =
        createEffect(rq->arena, getTexturedQuadShader(rq->ctx->internalCtx), EFFECT_BLEND_ALPHA_BLEND);
    rendererSetEffectTexture(effect, 0, textureHandle);
    pushQuads(rq, &quad, 1, effect, isTopDown);
}
RENDERER_PUSH_COLORED_QUAD(rendererPushColoredQuad)
{
    RenderEffect *effect =
        createEffect(rq->arena, getColoredQuadShader(rq->ctx->internalCtx), EFFECT_BLEND_ALPHA_BLEND);
    rendererSetEffectVec3(effect, "color", color);
    pushQuads(rq, &quad, 1, effect, true);
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
    cmd->instanceOffset = rq->meshInstanceCount;
    cmd->instanceCount = instanceCount;

    cmd->mesh = {0};
    if (mesh)
    {
        LoadedAsset *meshAsset = assetsGetMesh(mesh);
        if (meshAsset->mesh)
        {
            cmd->mesh = meshAsset->mesh->handle;
        }
    }

    memcpy(rq->ctx->meshInstances + rq->meshInstanceCount, instances, sizeof(RenderMeshInstance) * instanceCount);
    rq->meshInstanceCount += instanceCount;
}

RENDERER_PUSH_TERRAIN(rendererPushTerrain)
{
    DrawTerrainCommand *cmd = pushRenderCommand(rq, DrawTerrainCommand);

    cmd->heightfield = heightfield;
    cmd->heightmapSize = heightmapSize;

    cmd->terrainShader = {0};
    if (terrainShader)
    {
        LoadedAsset *shader = assetsGetShader(terrainShader);
        if (shader->shader)
        {
            cmd->terrainShader = shader->shader->handle;
        }
    }

    cmd->heightmapTexture = heightmapTexture;
    cmd->referenceHeightmapTexture = referenceHeightmapTexture;
    cmd->xAdjacentHeightmapTexture = xAdjacentHeightmapTexture;
    cmd->xAdjacentReferenceHeightmapTexture = xAdjacentReferenceHeightmapTexture;
    cmd->yAdjacentHeightmapTexture = yAdjacentHeightmapTexture;
    cmd->yAdjacentReferenceHeightmapTexture = yAdjacentReferenceHeightmapTexture;
    cmd->oppositeHeightmapTexture = oppositeHeightmapTexture;
    cmd->oppositeReferenceHeightmapTexture = oppositeReferenceHeightmapTexture;

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

RENDERER_DRAW_TO_TARGET(rendererDrawToTarget)
{
    return drawToTarget(rq, target->width, target->height, target);
}
RENDERER_DRAW_TO_SCREEN(rendererDrawToScreen)
{
    return drawToTarget(rq, width, height, 0);
}