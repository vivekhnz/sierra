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

RENDERER_CREATE_EFFECT_OVERRIDE(rendererCreateEffectOverride)
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

RENDERER_SET_EFFECT_FLOAT(rendererSetEffectFloat)
{
    RenderEffectParameter *param = pushEffectParameter(effect, paramName);
    param->type = EFFECT_PARAM_TYPE_FLOAT;
    param->value.f = value;
}
RENDERER_SET_EFFECT_VEC2(rendererSetEffectVec2)
{
    RenderEffectParameter *param = pushEffectParameter(effect, paramName);
    param->type = EFFECT_PARAM_TYPE_VEC2;
    param->value.v2 = value;
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

RENDERER_CREATE_QUEUE(rendererCreateQueue)
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
RENDERER_PUSH_QUAD_BOTTOM_UP(rendererPushQuadBottomUp)
{
    pushQuads(rq, &quad, 1, effect, false);
}
RENDERER_PUSH_QUADS(rendererPushQuads)
{
    pushQuads(rq, quads, quadCount, effect, true);
}

inline void addPrimitiveVertex(RenderQueue *rq, glm::vec3 point)
{
    assert(rq->primitiveVertexCount < rq->maxPrimitiveVertices + 1);
    rq->primitiveVertices[rq->primitiveVertexCount++] = point;
}
RENDERER_PUSH_LINE(rendererPushLine)
{
    DrawLineCommand *cmd = pushRenderCommand(rq, DrawLineCommand);
    cmd->vertexIndex = rq->primitiveVertexCount;
    cmd->vertexCount = 2;
    cmd->color = color;

    addPrimitiveVertex(rq, start);
    addPrimitiveVertex(rq, end);
}
RENDERER_BEGIN_LINE(rendererBeginLine)
{
    assert(!rq->currentLine.isActive);
    rq->currentLine.isActive = true;
    rq->currentLine.color = color;

    rq->currentLine.firstVertex = pushStruct(rq->arena, VertexLink);
    rq->currentLine.firstVertex->point = start;
    rq->currentLine.firstVertex->next = 0;
    rq->currentLine.lastVertex = rq->currentLine.firstVertex;
}
RENDERER_EXTEND_LINE(rendererExtendLine)
{
    assert(rq->currentLine.isActive);

    VertexLink *newLink = pushStruct(rq->arena, VertexLink);
    newLink->point = point;
    newLink->next = 0;

    rq->currentLine.lastVertex->next = newLink;
    rq->currentLine.lastVertex = newLink;
}
RENDERER_END_LINE(rendererEndLine)
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
RENDERER_END_LINE_LOOP(rendererEndLineLoop)
{
    assert(rq->currentLine.isActive);
    rendererEndLine(rq, rq->currentLine.firstVertex->point);
}

RENDERER_PUSH_QUAD_OUTLINE_XY(rendererPushQuadOutlineXy)
{
    rendererBeginLine(rq, glm::vec3(quad.x, quad.y, 0), color);
    rendererExtendLine(rq, glm::vec3(quad.x + quad.width, quad.y, 0));
    rendererExtendLine(rq, glm::vec3(quad.x + quad.width, quad.y + quad.height, 0));
    rendererExtendLine(rq, glm::vec3(quad.x, quad.y + quad.height, 0));
    rendererEndLineLoop(rq);
}
RENDERER_PUSH_QUAD_OUTLINE_XZ(rendererPushQuadOutlineXz)
{
    rendererBeginLine(rq, glm::vec3(quad.x, 0, quad.y), color);
    rendererExtendLine(rq, glm::vec3(quad.x + quad.width, 0, quad.y));
    rendererExtendLine(rq, glm::vec3(quad.x + quad.width, 0, quad.y + quad.height));
    rendererExtendLine(rq, glm::vec3(quad.x, 0, quad.y + quad.height));
    rendererEndLineLoop(rq);
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