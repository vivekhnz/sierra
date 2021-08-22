#include "engine_renderer.h"
#include "engine_render_backend.h"

ASSETS_GET_SHADER(assetsGetShader);
ASSETS_GET_TEXTURE(assetsGetTexture);
ASSETS_GET_MESH(assetsGetMesh);

struct RenderContext
{
    MemoryArena *arena;
    RenderBackendContext internalCtx;
};
struct RenderQueue
{
    MemoryArena *arena;
    RenderContext *ctx;
    RenderOutput output;

    RenderQueueCommandHeader *firstCommand;
    RenderQueueCommandHeader *lastCommand;

    RenderQuad *quads;
    uint32 maxQuads;
    uint32 quadCount;

    glm::vec3 *primitiveVertices;
    uint32 maxPrimitiveVertices;
    uint32 primitiveVertexCount;

    RenderMeshInstance *meshInstances;
    uint32 maxMeshInstances;
    uint32 meshInstanceCount;
};

RENDERER_INITIALIZE(rendererInitialize)
{
    RenderContext *ctx = pushStruct(arena, RenderContext);
    *ctx = {};
    ctx->arena = arena;
    ctx->internalCtx = initializeRenderBackend(arena);

    return ctx;
}

// textures

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
    return getPixels(arena, handle, width, height);
}
RENDERER_GET_PIXELS_IN_REGION(rendererGetPixelsInRegion)
{
    return getPixelsInRegion(arena, handle, x, y, width, height);
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
    result->output = output;

    result->maxQuads = 65536;
    result->quads = pushArray(arena, RenderQuad, result->maxQuads);

    result->maxPrimitiveVertices = 4096;
    result->primitiveVertices = pushArray(arena, glm::vec3, result->maxPrimitiveVertices);

    result->maxMeshInstances = 4096;
    result->meshInstances = pushArray(arena, RenderMeshInstance, result->maxMeshInstances);

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

glm::mat4 getOrthoTransform(RenderOutput *output, glm::vec2 cameraPos)
{
    // map from ([0 - width], [0 - height]) -> ([-1 - 1], [-1 - 1])
    glm::mat4 result = glm::identity<glm::mat4>();
    result = glm::scale(result, glm::vec3(2.0f / output->width, 2.0f / output->height, 1));
    result = glm::translate(
        result, glm::vec3(-((output->width * 0.5f) + cameraPos.x), -((output->height * 0.5f) + cameraPos.y), 0));

    return result;
}
RENDERER_SET_CAMERA_ORTHO(rendererSetCameraOrtho)
{
    SetCameraCommand *cmd = pushRenderCommand(rq, SetCameraCommand);
    cmd->transform = getOrthoTransform(&rq->output, glm::vec2(0, 0));

    return cmd->transform;
}
RENDERER_SET_CAMERA_ORTHO_OFFSET(rendererSetCameraOrthoOffset)
{
    SetCameraCommand *cmd = pushRenderCommand(rq, SetCameraCommand);
    cmd->transform = getOrthoTransform(&rq->output, cameraPos);

    return cmd->transform;
}
RENDERER_SET_CAMERA_PERSP(rendererSetCameraPersp)
{
    SetCameraCommand *cmd = pushRenderCommand(rq, SetCameraCommand);
    float nearPlane = 0.1f;
    float farPlane = 100000;
    glm::vec3 up = glm::vec3(0, 1, 0);
    float aspectRatio = (float)rq->output.width / (float)rq->output.height;
    glm::mat4 projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
    cmd->transform = projection * glm::lookAt(cameraPos, lookAt, up);

    return cmd->transform;
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
        assert(rq->quadCount + quadCount < rq->maxQuads + 1);

        DrawQuadsCommand *cmd = pushRenderCommand(rq, DrawQuadsCommand);
        cmd->effect = effect;
        cmd->isTopDown = isTopDown;
        cmd->instanceOffset = rq->quadCount;
        cmd->instanceCount = quadCount;

        memcpy(rq->quads + rq->quadCount, quads, sizeof(RenderQuad) * quadCount);
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

RENDERER_PUSH_LINE(rendererPushLine)
{
    assert(rq->primitiveVertexCount + 1 < rq->maxPrimitiveVertices);

    DrawLineCommand *cmd = pushRenderCommand(rq, DrawLineCommand);
    cmd->vertexIndex = rq->primitiveVertexCount;
    cmd->color = color;

    rq->primitiveVertices[rq->primitiveVertexCount++] = start;
    rq->primitiveVertices[rq->primitiveVertexCount++] = end;
}

RENDERER_PUSH_MESHES(rendererPushMeshes)
{
    assert(rq->meshInstanceCount + instanceCount < rq->maxMeshInstances + 1);

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

    memcpy(rq->meshInstances + rq->meshInstanceCount, instances, sizeof(RenderMeshInstance) * instanceCount);
    rq->meshInstanceCount += instanceCount;
}

TextureAsset *getTexture(AssetHandle assetHandle)
{
    TextureAsset *result = 0;
    if (assetHandle)
    {
        LoadedAsset *asset = assetsGetTexture(assetHandle);
        result = asset->texture;
    }
    return result;
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

    cmd->materialCount = materialCount;
    cmd->materials = pushArray(rq->arena, ResolvedTerrainMaterial, cmd->materialCount);
    for (uint32 i = 0; i < cmd->materialCount; i++)
    {
        RenderTerrainMaterial *src = &materials[i];
        ResolvedTerrainMaterial *dst = &cmd->materials[i];

        dst->textureSizeInWorldUnits = src->textureSizeInWorldUnits;
        dst->albedoTexture = getTexture(src->albedoTexture);
        dst->normalTexture = getTexture(src->normalTexture);
        dst->displacementTexture = getTexture(src->displacementTexture);
        dst->aoTexture = getTexture(src->aoTexture);
        dst->slopeStart = src->slopeStart;
        dst->slopeEnd = src->slopeEnd;
        dst->altitudeStart = src->altitudeStart;
        dst->altitudeEnd = src->altitudeEnd;
    }

    cmd->isWireframe = isWireframe;

    cmd->visualizationMode = visualizationMode;
    cmd->cursorPos = cursorPos;
    cmd->cursorRadius = cursorRadius;
    cmd->cursorFalloff = cursorFalloff;
}

RENDERER_DRAW(rendererDraw)
{
    DispatchedRenderQueue dispatched;
    dispatched.ctx = rq->ctx->internalCtx;
    dispatched.quads = rq->quads;
    dispatched.quadCount = rq->quadCount;
    dispatched.primitiveVertices = rq->primitiveVertices;
    dispatched.primitiveVertexCount = rq->primitiveVertexCount;
    dispatched.meshInstances = rq->meshInstances;
    dispatched.meshInstanceCount = rq->meshInstanceCount;
    dispatched.firstCommand = rq->firstCommand;

    return drawToOutput(&dispatched, &rq->output);
}