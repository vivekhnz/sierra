#include "engine_renderer.h"

#include "engine_assets.h"
#include "engine_heightfield.h"

#define RENDERER_CAMERA_UBO_SLOT 0
#define RENDERER_LIGHTING_UBO_SLOT 1

extern EnginePlatformApi Platform;

ASSETS_GET_SHADER_PROGRAM(assetsGetShaderProgram);
ASSETS_GET_MESH(assetsGetMesh);

struct RenderContext
{
    uint32 globalVertexArrayId;

    AssetHandle quadShaderProgramHandle;
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
    EFFECT_PARAM_TYPE_INT
};
struct RenderEffectParameter
{
    char *name;
    RenderEffectParameterType type;
    union
    {
        float f;
        int32 i;
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
    AssetHandle shaderProgramHandle;
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

    AssetHandle calcTessLevelShaderProgram;
    AssetHandle terrainShaderProgram;

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
    uint32 cpuFormat;
    uint32 gpuFormat;
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

bool createShaderProgram(int shaderCount, uint32 *shaderIds, uint32 *out_id)
{
    uint32 id = glCreateProgram();
    for (int i = 0; i < shaderCount; i++)
    {
        glAttachShader(id, shaderIds[i]);
    }

    glLinkProgram(id);
    int32 succeeded;
    glGetProgramiv(id, GL_LINK_STATUS, &succeeded);
    if (succeeded)
    {
        for (int i = 0; i < shaderCount; i++)
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

RenderMesh *createMesh(
    MemoryArena *arena, void *vertices, uint32 vertexCount, void *indices, uint32 indexCount)
{
    RenderMesh *result = pushStruct(arena, RenderMesh);

    uint32 vertexBufferStride = 6 * sizeof(float);
    glGenBuffers(1, &result->vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, result->vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexBufferStride, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &result->elementBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result->elementBufferId);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(uint32), indices, GL_STATIC_DRAW);

    return result;
}

RENDERER_INITIALIZE(rendererInitialize)
{
    RenderContext *ctx = pushStruct(arena, RenderContext);
    ctx->quadShaderProgramHandle = quadShaderProgramHandle;

    glGenVertexArrays(1, &ctx->globalVertexArrayId);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    // initialize camera state
    glGenBuffers(1, &ctx->cameraUniformBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, ctx->cameraUniformBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuCameraState), 0, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, RENDERER_CAMERA_UBO_SLOT, ctx->cameraUniformBufferId,
        0, sizeof(GpuCameraState));

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
    glBindBufferRange(GL_UNIFORM_BUFFER, RENDERER_LIGHTING_UBO_SLOT,
        ctx->lightingUniformBufferId, 0, sizeof(lighting));

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
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(quadBottomUpVerts), quadBottomUpVerts, GL_STATIC_DRAW);

    uint32 quadIndices[6] = {0, 1, 2, 0, 2, 3};
    glGenBuffers(1, &ctx->quadElementBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->quadElementBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glGenBuffers(1, &ctx->quadInstanceBufferId);
    ctx->maxQuads = 65536;
    ctx->quads = (RenderQuad *)pushSize(arena, sizeof(RenderQuad) * ctx->maxQuads);

    glGenBuffers(1, &ctx->meshInstanceBufferId);
    ctx->maxMeshInstances = 4096;
    ctx->meshInstances = (RenderMeshInstance *)pushSize(
        arena, sizeof(RenderMeshInstance) * ctx->maxMeshInstances);

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
    glTexImage2D(
        GL_TEXTURE_2D, 0, cpuFormat, width, height, 0, gpuFormat, elementType, pixels);
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
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, cpuFormat, width, height, layers, 0, gpuFormat,
        elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    return id;
}

RENDERER_UPDATE_TEXTURE_ARRAY(rendererUpdateTextureArray)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glTexSubImage3D(
        GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, width, height, 1, gpuFormat, elementType, pixels);
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
        result.cpuFormat = GL_RGB;
        result.gpuFormat = GL_RGB;
        result.hasDepthBuffer = true;
    }
    else if (format == RENDER_TARGET_FORMAT_R16)
    {
        result.elementType = GL_UNSIGNED_SHORT;
        result.cpuFormat = GL_R16;
        result.gpuFormat = GL_RED;
        result.hasDepthBuffer = false;
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, descriptor.cpuFormat, width, height, 0,
        descriptor.gpuFormat, descriptor.elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    // create depth buffer
    if (descriptor.hasDepthBuffer)
    {
        glGenRenderbuffers(1, &result->depthBufferId);
        glBindRenderbuffer(GL_RENDERBUFFER, result->depthBufferId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    // create framebuffer
    glGenFramebuffers(1, &result->framebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, result->framebufferId);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result->textureId, 0);
    if (descriptor.hasDepthBuffer)
    {
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result->depthBufferId);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return result;
}

RENDERER_RESIZE_RENDER_TARGET(rendererResizeRenderTarget)
{
    target->width = width;
    target->height = height;

    RenderTargetDescriptor descriptor = getRenderTargetDescriptor(target->format);

    glBindTexture(GL_TEXTURE_2D, target->textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, descriptor.cpuFormat, width, height, 0,
        descriptor.gpuFormat, descriptor.elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindRenderbuffer(GL_RENDERBUFFER, target->depthBufferId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

// effects

RENDERER_CREATE_EFFECT(rendererCreateEffect)
{
    RenderEffect *result = pushStruct(arena, RenderEffect);
    *result = {};

    result->arena = arena;
    result->shaderProgramHandle = shaderProgram;
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
#define pushRenderCommand(rq, type)                                                           \
    (type *)pushRenderCommandInternal(rq, RENDER_CMD_##type, sizeof(type))

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

void pushQuads(
    RenderQueue *rq, RenderQuad *quads, uint32 quadCount, RenderEffect *effect, bool isTopDown)
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
    RenderEffect *effect = rendererCreateEffect(
        rq->arena, rq->ctx->quadShaderProgramHandle, EFFECT_BLEND_ALPHA_BLEND);
    rendererSetEffectTexture(effect, 0, textureId);
    pushQuads(rq, &quad, 1, effect, isTopDown);
}

RENDERER_PUSH_EFFECT_QUAD(rendererPushEffectQuad)
{
    pushQuads(rq, &quad, 1, effect, true);
}
RENDERER_PUSH_EFFECT_QUADS(rendererPushEffectQuads)
{
    pushQuads(rq, quads, quadCount, effect, true);
}

RENDERER_PUSH_MESHES(rendererPushMeshes)
{
    assert(rq->meshInstanceCount + instanceCount < rq->ctx->maxMeshInstances);

    RenderEffect *effect =
        rendererCreateEffect(rq->arena, shaderProgram, EFFECT_BLEND_ALPHA_BLEND);

    DrawMeshesCommand *cmd = pushRenderCommand(rq, DrawMeshesCommand);
    cmd->effect = effect;
    cmd->mesh = mesh;
    cmd->instanceOffset = rq->meshInstanceCount;
    cmd->instanceCount = instanceCount;

    memcpy(rq->ctx->meshInstances + rq->meshInstanceCount, instances,
        sizeof(RenderMeshInstance) * instanceCount);
    rq->meshInstanceCount += instanceCount;
}

RENDERER_PUSH_TERRAIN(rendererPushTerrain)
{
    DrawTerrainCommand *cmd = pushRenderCommand(rq, DrawTerrainCommand);

    cmd->heightfield = heightfield;

    cmd->calcTessLevelShaderProgram = calcTessLevelShaderProgram;
    cmd->terrainShaderProgram = terrainShaderProgram;

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
    bool wasEffectApplied = false;

    LoadedAsset *shaderProgram = assetsGetShaderProgram(effect->shaderProgramHandle);
    if (shaderProgram->shaderProgram)
    {
        uint32 shaderProgramId = shaderProgram->shaderProgram->id;
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
            {
                glProgramUniform1f(shaderProgramId, loc, effectParam->value.f);
            }
            break;
            case EFFECT_PARAM_TYPE_INT:
            {
                glProgramUniform1i(shaderProgramId, loc, effectParam->value.i);
            }
            break;
            default:
            {
                assert(!"Invalid effect parameter type");
            }
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

        wasEffectApplied = true;
    }

    return wasEffectApplied;
}

bool drawToTarget(RenderQueue *rq, uint32 width, uint32 height, RenderTarget *target)
{
    if (target)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, target->framebufferId);
    }
    glViewport(0, 0, width, height);

    glBindBuffer(GL_ARRAY_BUFFER, rq->ctx->quadInstanceBufferId);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(RenderQuad) * rq->quadCount, rq->ctx->quads, GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, rq->ctx->meshInstanceBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RenderMeshInstance) * rq->meshInstanceCount,
        rq->ctx->meshInstances, GL_STREAM_DRAW);

    glBindVertexArray(rq->ctx->globalVertexArrayId);

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
                camera.transform =
                    glm::translate(camera.transform, glm::vec3(-0.5f, -0.5f, 0));
            }
            else
            {
                float nearPlane = 0.1f;
                float farPlane = 10000;
                glm::vec3 up = glm::vec3(0, 1, 0);
                float aspectRatio = (float)width / (float)height;
                glm::mat4 projection =
                    glm::perspective(cmd->fov, aspectRatio, nearPlane, farPlane);
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
                uint32 vertexBufferId = cmd->isTopDown ? rq->ctx->quadTopDownVertexBufferId
                                                       : rq->ctx->quadBottomUpVertexBufferId;
                uint32 vertexBufferStride = 4 * sizeof(float);
                glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rq->ctx->quadElementBufferId);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 2, GL_FLOAT, false, vertexBufferStride, (void *)0);
                glVertexAttribPointer(
                    1, 2, GL_FLOAT, false, vertexBufferStride, (void *)(2 * sizeof(float)));

                uint32 instanceBufferStride = 4 * sizeof(float);
                glBindBuffer(GL_ARRAY_BUFFER, rq->ctx->quadInstanceBufferId);
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 4, GL_FLOAT, false, instanceBufferStride, (void *)0);
                glVertexAttribDivisor(2, 1);

                glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0,
                    cmd->instanceCount, cmd->instanceOffset);

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
                glVertexAttribPointer(
                    1, 3, GL_FLOAT, false, vertexBufferStride, (void *)(3 * sizeof(float)));

                uint32 instanceBufferStride = sizeof(glm::mat4);
                glBindBuffer(GL_ARRAY_BUFFER, rq->ctx->meshInstanceBufferId);
                glEnableVertexAttribArray(2);
                glEnableVertexAttribArray(3);
                glEnableVertexAttribArray(4);
                glEnableVertexAttribArray(5);
                glVertexAttribPointer(2, 4, GL_FLOAT, false, instanceBufferStride, (void *)0);
                glVertexAttribDivisor(2, 1);
                glVertexAttribPointer(
                    3, 4, GL_FLOAT, false, instanceBufferStride, (void *)(4 * sizeof(float)));
                glVertexAttribDivisor(3, 1);
                glVertexAttribPointer(
                    4, 4, GL_FLOAT, false, instanceBufferStride, (void *)(8 * sizeof(float)));
                glVertexAttribDivisor(4, 1);
                glVertexAttribPointer(
                    5, 4, GL_FLOAT, false, instanceBufferStride, (void *)(12 * sizeof(float)));
                glVertexAttribDivisor(5, 1);

                glDrawElementsInstancedBaseInstance(GL_TRIANGLES, mesh->elementCount,
                    GL_UNSIGNED_INT, 0, cmd->instanceCount, cmd->instanceOffset);

                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                glDisableVertexAttribArray(2);
                glDisableVertexAttribArray(3);
                glDisableVertexAttribArray(4);
                glDisableVertexAttribArray(5);
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

            LoadedAsset *calcTessLevelShaderProgram =
                assetsGetShaderProgram(cmd->calcTessLevelShaderProgram);
            LoadedAsset *terrainShaderProgram =
                assetsGetShaderProgram(cmd->terrainShaderProgram);
            if (calcTessLevelShaderProgram->shaderProgram
                && terrainShaderProgram->shaderProgram)
            {
                Heightfield *heightfield = cmd->heightfield;
                uint32 calcTessLevelShaderProgramId =
                    calcTessLevelShaderProgram->shaderProgram->id;
                uint32 terrainShaderProgramId = terrainShaderProgram->shaderProgram->id;
                uint32 meshEdgeCount = (2 * (heightfield->rows * heightfield->columns))
                    - heightfield->rows - heightfield->columns;
                glm::vec3 terrainDimensions =
                    glm::vec3(heightfield->spacing * heightfield->columns,
                        heightfield->maxHeight, heightfield->spacing * heightfield->rows);

                // calculate tessellation levels
                glUseProgram(calcTessLevelShaderProgramId);
                glProgramUniform1f(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "targetTriangleSize"),
                    0.015f);
                glProgramUniform1i(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "horizontalEdgeCount"),
                    heightfield->rows * (heightfield->columns - 1));
                glProgramUniform1i(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "columnCount"),
                    heightfield->columns);
                glProgramUniform1f(calcTessLevelShaderProgramId,
                    glGetUniformLocation(calcTessLevelShaderProgramId, "terrainHeight"),
                    heightfield->maxHeight);
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
                glProgramUniform1i(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "materialCount"),
                    cmd->materialCount);
                glProgramUniform3fv(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "terrainDimensions"), 1,
                    glm::value_ptr(terrainDimensions));
                glProgramUniform1i(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "visualizationMode"),
                    cmd->visualizationMode);
                glProgramUniform2fv(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "cursorPos"), 1,
                    glm::value_ptr(cmd->cursorPos));
                glProgramUniform1f(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "cursorRadius"),
                    cmd->cursorRadius);
                glProgramUniform1f(terrainShaderProgramId,
                    glGetUniformLocation(terrainShaderProgramId, "cursorFalloff"),
                    cmd->cursorFalloff);

                uint32 vertexBufferStride = 5 * sizeof(float);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd->meshElementBufferId);
                glBindBuffer(GL_ARRAY_BUFFER, cmd->meshVertexBufferId);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 3, GL_FLOAT, false, vertexBufferStride, 0);
                glVertexAttribPointer(
                    1, 2, GL_FLOAT, false, vertexBufferStride, (void *)(3 * sizeof(float)));

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