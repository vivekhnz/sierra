#include "engine_renderer.h"
#include "engine_render_backend.h"

LoadedAsset *assetsGetShader(AssetHandle assetHandle);
LoadedAsset *assetsGetTexture(AssetHandle assetHandle);
LoadedAsset *assetsGetMesh(AssetHandle assetHandle);

struct RenderContext
{
    MemoryArena *arena;
    RenderBackendContext internalCtx;
};
struct VertexLink
{
    glm::vec3 point;
    VertexLink *next;
};
struct RenderQueue
{
    MemoryArena *arena;
    RenderContext *ctx;
    RenderOutput output;

    RenderQueueCommandHeader *firstCommand;
    RenderQueueCommandHeader *lastCommand;

    rect2 *quads;
    uint32 maxQuads;
    uint32 quadCount;

    glm::vec3 *primitiveVertices;
    uint32 maxPrimitiveVertices;
    uint32 primitiveVertexCount;

    RenderMeshInstance *meshInstances;
    uint32 maxMeshInstances;
    uint32 meshInstanceCount;

    struct
    {
        bool isActive;
        glm::vec3 color;
        VertexLink *firstVertex;
        VertexLink *lastVertex;
    } currentLine;
};

RenderContext *rendererInitialize(MemoryArena *arena)
{
    RenderContext *ctx = pushStruct(arena, RenderContext);
    *ctx = {};
    ctx->arena = arena;
    ctx->internalCtx = initializeRenderBackend(arena);

    return ctx;
}

// textures

TextureHandle rendererCreateTexture(uint32 width, uint32 height, TextureFormat format)
{
    return createTexture(width, height, format);
}
void rendererUpdateTexture(TextureHandle handle, uint32 width, uint32 height, void *pixels)
{
    updateTexture(handle, width, height, pixels);
}
GetPixelsResult rendererGetPixels(MemoryArena *arena, TextureHandle handle, uint32 width, uint32 height)
{
    return getPixels(arena, handle, width, height);
}
GetPixelsResult rendererGetPixelsInRegion(
    MemoryArena *arena, TextureHandle handle, uint32 x, uint32 y, uint32 width, uint32 height)
{
    return getPixelsInRegion(arena, handle, x, y, width, height);
}

// render targets

RenderTarget *rendererCreateRenderTarget(
    MemoryArena *arena, uint32 width, uint32 height, TextureFormat format, bool createDepthBuffer)
{
    return createRenderTarget(arena, width, height, format, createDepthBuffer);
}
void rendererResizeRenderTarget(RenderTarget *target, uint32 width, uint32 height)
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

    return result;
}

RenderEffect *rendererCreateEffect(MemoryArena *arena, AssetHandle shaderAsset, RenderEffectBlendMode blendMode)
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

RenderEffect *rendererCreateEffectOverride(RenderEffect *baseEffect)
{
    MemoryArena *arena = baseEffect->arena;
    RenderEffect *result = pushStruct(arena, RenderEffect);
    *result = {};

    result->arena = arena;
    result->shaderHandle = baseEffect->shaderHandle;
    result->blendMode = baseEffect->blendMode;

    if (baseEffect->firstParameter)
    {
        result->firstParameter = pushStruct(arena, RenderEffectParameterLink);
        *result->firstParameter = {};
        result->firstParameter->param = baseEffect->firstParameter->param;

        RenderEffectParameterLink *last = result->firstParameter;
        for (RenderEffectParameterLink *link = baseEffect->firstParameter->next; link; link = link->next)
        {
            RenderEffectParameterLink *copy = pushStruct(arena, RenderEffectParameterLink);
            *copy = {};
            copy->param = link->param;

            last->next = copy;
            last = copy;
        }
        result->lastParameter = last;
    }
    if (baseEffect->firstTexture)
    {
        result->firstTexture = pushStruct(arena, RenderEffectTextureLink);
        *result->firstTexture = {};
        result->firstTexture->texture = baseEffect->firstTexture->texture;

        RenderEffectTextureLink *last = result->firstTexture;
        for (RenderEffectTextureLink *link = baseEffect->firstTexture->next; link; link = link->next)
        {
            RenderEffectTextureLink *copy = pushStruct(arena, RenderEffectTextureLink);
            *copy = {};
            copy->texture = link->texture;

            last->next = copy;
            last = copy;
        }
        result->lastTexture = last;
    }

    return result;
}

inline RenderEffectParameter *pushEffectParameter(RenderEffect *effect, char *paramName)
{
    RenderEffectParameter *param = pushStruct(effect->arena, RenderEffectParameter);
    *param = {};

    RenderEffectParameterLink *link = pushStruct(effect->arena, RenderEffectParameterLink);
    *link = {};
    link->param = param;

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
        effect->lastParameter->next = link;
    }
    effect->lastParameter = link;
    if (!effect->firstParameter)
    {
        effect->firstParameter = link;
    }

    return param;
}

void rendererSetEffectFloat(RenderEffect *effect, char *paramName, float value)
{
    RenderEffectParameter *param = pushEffectParameter(effect, paramName);
    param->type = EFFECT_PARAM_TYPE_FLOAT;
    param->value.f = value;
}
void rendererSetEffectVec2(RenderEffect *effect, char *paramName, glm::vec2 value)
{
    RenderEffectParameter *param = pushEffectParameter(effect, paramName);
    param->type = EFFECT_PARAM_TYPE_VEC2;
    param->value.v2 = value;
}
void rendererSetEffectVec3(RenderEffect *effect, char *paramName, glm::vec3 value)
{
    RenderEffectParameter *param = pushEffectParameter(effect, paramName);
    param->type = EFFECT_PARAM_TYPE_VEC3;
    param->value.v3 = value;
}
void rendererSetEffectInt(RenderEffect *effect, char *paramName, int32 value)
{
    RenderEffectParameter *param = pushEffectParameter(effect, paramName);
    param->type = EFFECT_PARAM_TYPE_INT;
    param->value.i = value;
}
void rendererSetEffectUint(RenderEffect *effect, char *paramName, uint32 value)
{
    RenderEffectParameter *param = pushEffectParameter(effect, paramName);
    param->type = EFFECT_PARAM_TYPE_UINT;
    param->value.u = value;
}

void rendererSetEffectTexture(RenderEffect *effect, uint32 slot, TextureHandle handle)
{
    RenderEffectTexture *texture = pushStruct(effect->arena, RenderEffectTexture);
    *texture = {};
    texture->slot = slot;
    texture->handle = handle;

    RenderEffectTextureLink *link = pushStruct(effect->arena, RenderEffectTextureLink);
    *link = {};
    link->texture = texture;

    if (effect->lastTexture)
    {
        effect->lastTexture->next = link;
    }
    effect->lastTexture = link;
    if (!effect->firstTexture)
    {
        effect->firstTexture = link;
    }
}

// render queue

RenderQueue *rendererCreateQueue(RenderContext *ctx, MemoryArena *arena, RenderOutput output)
{
    RenderQueue *result = pushStruct(arena, RenderQueue);
    *result = {};
    result->arena = arena;
    result->ctx = ctx;
    result->output = output;

    result->maxQuads = 65536;
    result->quads = pushArray(arena, rect2, result->maxQuads);

    result->maxPrimitiveVertices = 1000000;
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
glm::mat4 rendererSetCameraOrtho(RenderQueue *rq)
{
    SetCameraCommand *cmd = pushRenderCommand(rq, SetCameraCommand);
    cmd->transform = getOrthoTransform(&rq->output, glm::vec2(0, 0));

    return cmd->transform;
}
glm::mat4 rendererSetCameraOrthoOffset(RenderQueue *rq, glm::vec2 cameraPos)
{
    SetCameraCommand *cmd = pushRenderCommand(rq, SetCameraCommand);
    cmd->transform = getOrthoTransform(&rq->output, cameraPos);

    return cmd->transform;
}
glm::mat4 rendererSetCameraPersp(RenderQueue *rq, glm::vec3 cameraPos, glm::vec3 lookAt, float fov)
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

void rendererSetLighting(RenderQueue *rq,
    glm::vec4 *lightDir,
    bool isLightingEnabled,
    bool isTextureEnabled,
    bool isNormalMapEnabled,
    bool isAOMapEnabled,
    bool isDisplacementMapEnabled)
{
    SetLightingCommand *cmd = pushRenderCommand(rq, SetLightingCommand);
    cmd->lightDir = *lightDir;
    cmd->isEnabled = isLightingEnabled;
    cmd->isTextureEnabled = isTextureEnabled;
    cmd->isNormalMapEnabled = isNormalMapEnabled;
    cmd->isAOMapEnabled = isAOMapEnabled;
    cmd->isDisplacementMapEnabled = isDisplacementMapEnabled;
}

void rendererClear(RenderQueue *rq, float r, float g, float b, float a)
{
    ClearCommand *cmd = pushRenderCommand(rq, ClearCommand);
    cmd->color.r = r;
    cmd->color.g = g;
    cmd->color.b = b;
    cmd->color.a = a;
}

void pushQuads(RenderQueue *rq, rect2 *quads, uint32 quadCount, RenderEffect *effect, bool isTopDown)
{
    if (quadCount > 0)
    {
        assert(rq->quadCount + quadCount < rq->maxQuads + 1);

        DrawQuadsCommand *cmd = pushRenderCommand(rq, DrawQuadsCommand);
        cmd->effect = effect;
        cmd->isTopDown = isTopDown;
        cmd->instanceOffset = rq->quadCount;
        cmd->instanceCount = quadCount;

        memcpy(rq->quads + rq->quadCount, quads, sizeof(rect2) * quadCount);
        rq->quadCount += quadCount;
    }
}

void rendererPushTexturedQuad(RenderQueue *rq, rect2 quad, TextureHandle textureHandle, bool isTopDown)
{
    RenderEffect *effect =
        createEffect(rq->arena, getTexturedQuadShader(rq->ctx->internalCtx), EFFECT_BLEND_ALPHA_BLEND);
    rendererSetEffectTexture(effect, 0, textureHandle);
    rendererSetEffectVec2(effect, "uvScale", glm::vec2(1, 1));
    rendererSetEffectVec2(effect, "uvOffset", glm::vec2(0, 0));
    pushQuads(rq, &quad, 1, effect, isTopDown);
}
void rendererPushTexturedQuadRegion(
    RenderQueue *rq, rect2 quad, TextureHandle textureHandle, bool isTopDown, rect2 uvRect)
{
    RenderEffect *effect =
        createEffect(rq->arena, getTexturedQuadShader(rq->ctx->internalCtx), EFFECT_BLEND_ALPHA_BLEND);
    rendererSetEffectTexture(effect, 0, textureHandle);
    rendererSetEffectVec2(effect, "uvScale", glm::vec2(uvRect.width, uvRect.height));
    rendererSetEffectVec2(effect, "uvOffset", glm::vec2(uvRect.x, uvRect.y));

    pushQuads(rq, &quad, 1, effect, isTopDown);
}
void rendererPushColoredQuad(RenderQueue *rq, rect2 quad, glm::vec3 color)
{
    RenderEffect *effect =
        createEffect(rq->arena, getColoredQuadShader(rq->ctx->internalCtx), EFFECT_BLEND_ALPHA_BLEND);
    rendererSetEffectVec3(effect, "color", color);
    pushQuads(rq, &quad, 1, effect, true);
}

void rendererPushQuad(RenderQueue *rq, rect2 quad, RenderEffect *effect)
{
    pushQuads(rq, &quad, 1, effect, true);
}
void rendererPushQuadBottomUp(RenderQueue *rq, rect2 quad, RenderEffect *effect)
{
    pushQuads(rq, &quad, 1, effect, false);
}
void rendererPushQuads(RenderQueue *rq, rect2 *quads, uint32 quadCount, RenderEffect *effect)
{
    pushQuads(rq, quads, quadCount, effect, true);
}

inline void addPrimitiveVertex(RenderQueue *rq, glm::vec3 point)
{
    assert(rq->primitiveVertexCount < rq->maxPrimitiveVertices + 1);
    rq->primitiveVertices[rq->primitiveVertexCount++] = point;
}
void rendererPushLine(RenderQueue *rq, glm::vec3 start, glm::vec3 end, glm::vec3 color)
{
    DrawLineCommand *cmd = pushRenderCommand(rq, DrawLineCommand);
    cmd->vertexIndex = rq->primitiveVertexCount;
    cmd->vertexCount = 2;
    cmd->color = color;

    addPrimitiveVertex(rq, start);
    addPrimitiveVertex(rq, end);
}
void rendererBeginLine(RenderQueue *rq, glm::vec3 start, glm::vec3 color)
{
    assert(!rq->currentLine.isActive);
    rq->currentLine.isActive = true;
    rq->currentLine.color = color;

    rq->currentLine.firstVertex = pushStruct(rq->arena, VertexLink);
    rq->currentLine.firstVertex->point = start;
    rq->currentLine.firstVertex->next = 0;
    rq->currentLine.lastVertex = rq->currentLine.firstVertex;
}
void rendererExtendLine(RenderQueue *rq, glm::vec3 point)
{
    assert(rq->currentLine.isActive);

    VertexLink *newLink = pushStruct(rq->arena, VertexLink);
    newLink->point = point;
    newLink->next = 0;

    rq->currentLine.lastVertex->next = newLink;
    rq->currentLine.lastVertex = newLink;
}
void rendererEndLine(RenderQueue *rq, glm::vec3 end)
{
    assert(rq->currentLine.isActive);
    rq->currentLine.isActive = false;

    DrawLineCommand *cmd = pushRenderCommand(rq, DrawLineCommand);
    cmd->vertexIndex = rq->primitiveVertexCount;
    cmd->vertexCount = 1;
    cmd->color = rq->currentLine.color;

    for (VertexLink *current = rq->currentLine.firstVertex; current; current = current->next)
    {
        addPrimitiveVertex(rq, current->point);
        cmd->vertexCount++;
    }
    addPrimitiveVertex(rq, end);
}
void rendererEndLineLoop(RenderQueue *rq)
{
    assert(rq->currentLine.isActive);
    rendererEndLine(rq, rq->currentLine.firstVertex->point);
}

void rendererPushQuadOutlineXy(RenderQueue *rq, rect2 quad, glm::vec3 color)
{
    rendererBeginLine(rq, glm::vec3(quad.x, quad.y, 0), color);
    rendererExtendLine(rq, glm::vec3(quad.x + quad.width, quad.y, 0));
    rendererExtendLine(rq, glm::vec3(quad.x + quad.width, quad.y + quad.height, 0));
    rendererExtendLine(rq, glm::vec3(quad.x, quad.y + quad.height, 0));
    rendererEndLineLoop(rq);
}
void rendererPushQuadOutlineXz(RenderQueue *rq, rect2 quad, glm::vec3 color)
{
    rendererBeginLine(rq, glm::vec3(quad.x, 0, quad.y), color);
    rendererExtendLine(rq, glm::vec3(quad.x + quad.width, 0, quad.y));
    rendererExtendLine(rq, glm::vec3(quad.x + quad.width, 0, quad.y + quad.height));
    rendererExtendLine(rq, glm::vec3(quad.x, 0, quad.y + quad.height));
    rendererEndLineLoop(rq);
}

void rendererPushMeshes(
    RenderQueue *rq, AssetHandle mesh, RenderMeshInstance *instances, uint32 instanceCount, RenderEffect *effect)
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
void rendererPushTerrain(RenderQueue *rq,
    glm::vec2 heightfieldCenter,
    float heightfieldMaxHeight,
    glm::vec2 heightmapSize,
    float heightmapOverlapInTexels,
    AssetHandle terrainShader,
    TextureHandle heightmapTexture,
    TextureHandle referenceHeightmapTexture,
    uint32 materialCount,
    RenderTerrainMaterial *materials,
    bool isWireframe,
    uint32 visualizationMode,
    glm::vec2 cursorPos,
    float cursorRadius,
    float cursorFalloff)
{
    DrawTerrainCommand *cmd = pushRenderCommand(rq, DrawTerrainCommand);

    cmd->heightfieldCenter = heightfieldCenter;
    cmd->heightfieldMaxHeight = heightfieldMaxHeight;
    cmd->heightmapSize = heightmapSize;
    cmd->heightmapOverlapInTexels = heightmapOverlapInTexels;

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

bool rendererDraw(RenderQueue *rq)
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