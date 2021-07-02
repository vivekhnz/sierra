#include "engine_renderer.h"

#include "engine_assets.h"

#define RENDERER_MAX_FRAMEBUFFERS 128
#define RENDERER_MAX_SHADER_PROGRAMS 128
#define RENDERER_MAX_VERTEX_ARRAYS 128
#define RENDERER_MAX_BUFFERS 128

extern EnginePlatformApi Platform;

ASSETS_GET_SHADER_PROGRAM(assetsGetShaderProgram);

enum RendererUniformBuffer
{
    RENDERER_UNIFORM_BUFFER_CAMERA,
    RENDERER_UNIFORM_BUFFER_LIGHTING,

    RENDERER_UNIFORM_BUFFER_COUNT
};

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

    uint32 framebufferCount;
    uint32 framebufferIds[RENDERER_MAX_FRAMEBUFFERS];
    uint32 framebufferTextureIds[RENDERER_MAX_FRAMEBUFFERS];

    uint32 vertexArrayCount;
    uint32 vertexArrayIds[RENDERER_MAX_VERTEX_ARRAYS];

    uint32 bufferCount;
    uint32 bufferIds[RENDERER_MAX_BUFFERS];
    RendererBufferType bufferTypes[RENDERER_MAX_BUFFERS];
    uint32 bufferUsages[RENDERER_MAX_BUFFERS];
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
    RENDER_CMD_DrawQuadsCommand
};
struct RenderQueueCommandHeader
{
    RenderQueueCommandType type;
    RenderQueueCommandHeader *next;
};

struct DrawQuadsCommand
{
    RenderEffect *effect;
    bool isTopDown;
    uint32 instanceOffset;
    uint32 instanceCount;
};
struct RenderQueue
{
    MemoryArena *arena;
    RenderContext *ctx;

    glm::mat4 cameraTransform;
    glm::vec4 clearColor;

    RenderQueueCommandHeader *firstCommand;
    RenderQueueCommandHeader *lastCommand;

    uint32 quadCount;
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

struct RenderTargetDescriptor
{
    uint32 elementType;
    uint32 cpuFormat;
    uint32 gpuFormat;
    bool hasDepthBuffer;
};

uint32 getOpenGLBufferType(RendererBufferType type)
{
    uint32 bufferType = 0;
    if (type == RENDERER_VERTEX_BUFFER)
    {
        bufferType = GL_ARRAY_BUFFER;
    }
    else if (type == RENDERER_ELEMENT_BUFFER)
    {
        bufferType = GL_ELEMENT_ARRAY_BUFFER;
    }
    else if (type == RENDERER_SHADER_STORAGE_BUFFER)
    {
        bufferType = GL_SHADER_STORAGE_BUFFER;
    }
    assert(bufferType);
    return bufferType;
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

    glGenBuffers(RENDERER_UNIFORM_BUFFER_COUNT, ctx->bufferIds);
    ctx->bufferCount = RENDERER_UNIFORM_BUFFER_COUNT;

    // initialize camera state
    uint32 cameraUboId = ctx->bufferIds[RENDERER_UNIFORM_BUFFER_CAMERA];
    glBindBuffer(GL_UNIFORM_BUFFER, cameraUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GpuCameraState), 0, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, RENDERER_UNIFORM_BUFFER_CAMERA, cameraUboId, 0,
        sizeof(GpuCameraState));

    // initialize lighting state
    GpuLightingState lighting;
    lighting.lightDir = glm::vec4(-0.588f, 0.809f, 0.294f, 0.0f);
    lighting.isEnabled = true;
    lighting.isTextureEnabled = true;
    lighting.isNormalMapEnabled = true;
    lighting.isAOMapEnabled = true;
    lighting.isDisplacementMapEnabled = true;

    uint32 lightingUboId = ctx->bufferIds[RENDERER_UNIFORM_BUFFER_LIGHTING];
    glBindBuffer(GL_UNIFORM_BUFFER, lightingUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(lighting), &lighting, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, RENDERER_UNIFORM_BUFFER_LIGHTING, lightingUboId, 0,
        sizeof(lighting));

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
    ctx->maxQuads = 1024;
    ctx->quads = (RenderQuad *)pushSize(arena, sizeof(RenderQuad) * ctx->maxQuads);

    return ctx;
}

RENDERER_UPDATE_CAMERA_STATE(rendererUpdateCameraState)
{
    GpuCameraState camera;
    camera.transform = *transform;

    glBindBuffer(GL_UNIFORM_BUFFER, ctx->bufferIds[RENDERER_UNIFORM_BUFFER_CAMERA]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera), &camera);
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

    glBindBuffer(GL_UNIFORM_BUFFER, ctx->bufferIds[RENDERER_UNIFORM_BUFFER_LIGHTING]);
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

RENDERER_BIND_TEXTURE(rendererBindTexture)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
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

RENDERER_BIND_TEXTURE_ARRAY(rendererBindTextureArray)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
}

RENDERER_UPDATE_TEXTURE_ARRAY(rendererUpdateTextureArray)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glTexSubImage3D(
        GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, width, height, 1, gpuFormat, elementType, pixels);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

RENDERER_CREATE_FRAMEBUFFER(rendererCreateFramebuffer)
{
    assert(ctx->framebufferCount < RENDERER_MAX_FRAMEBUFFERS);
    uint32 *framebufferId = ctx->framebufferIds + ctx->framebufferCount;
    glGenFramebuffers(1, framebufferId);

    glBindFramebuffer(GL_FRAMEBUFFER, *framebufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ctx->framebufferTextureIds[ctx->framebufferCount] = textureId;

    return ctx->framebufferCount++;
}

RENDERER_BIND_FRAMEBUFFER(rendererBindFramebuffer)
{
    assert(handle < ctx->framebufferCount);
    uint32 id = ctx->framebufferIds[handle];

    glBindFramebuffer(GL_FRAMEBUFFER, id);
}

RENDERER_UNBIND_FRAMEBUFFER(rendererUnbindFramebuffer)
{
    assert(handle < ctx->framebufferCount);

    uint32 textureId = ctx->framebufferTextureIds[handle];
    glBindTexture(GL_TEXTURE_2D, textureId);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RENDERER_CREATE_SHADER(rendererCreateShader)
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

RENDERER_CREATE_SHADER_PROGRAM(rendererCreateShaderProgram)
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

RENDERER_USE_SHADER_PROGRAM(rendererUseShaderProgram)
{
    glUseProgram(id);
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_FLOAT(rendererSetShaderProgramUniformFloat)
{
    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform1f(id, loc, value);
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_INTEGER(rendererSetShaderProgramUniformInteger)
{
    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform1i(id, loc, value);
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR2(rendererSetShaderProgramUniformVector2)
{
    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform2fv(id, loc, 1, glm::value_ptr(value));
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR3(rendererSetShaderProgramUniformVector3)
{
    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform3fv(id, loc, 1, glm::value_ptr(value));
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR4(rendererSetShaderProgramUniformVector4)
{
    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform4fv(id, loc, 1, glm::value_ptr(value));
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_MATRIX4X4(rendererSetShaderProgramUniformMatrix4x4)
{
    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniformMatrix4fv(id, loc, 1, false, glm::value_ptr(value));
}

RENDERER_CREATE_VERTEX_ARRAY(rendererCreateVertexArray)
{
    assert(ctx->vertexArrayCount < RENDERER_MAX_VERTEX_ARRAYS);
    glGenVertexArrays(1, ctx->vertexArrayIds + ctx->vertexArrayCount);
    return ctx->vertexArrayCount++;
}

RENDERER_BIND_VERTEX_ARRAY(rendererBindVertexArray)
{
    assert(handle < ctx->vertexArrayCount);
    uint32 id = ctx->vertexArrayIds[handle];

    glBindVertexArray(id);
}

RENDERER_UNBIND_VERTEX_ARRAY(rendererUnbindVertexArray)
{
    glBindVertexArray(0);
}

RENDERER_CREATE_BUFFER(rendererCreateBuffer)
{
    assert(ctx->bufferCount < RENDERER_MAX_BUFFERS);
    glGenBuffers(1, ctx->bufferIds + ctx->bufferCount);
    ctx->bufferTypes[ctx->bufferCount] = type;
    ctx->bufferUsages[ctx->bufferCount] = usage;
    return ctx->bufferCount++;
}

RENDERER_BIND_BUFFER(rendererBindBuffer)
{
    assert(handle < ctx->bufferCount);
    uint32 id = ctx->bufferIds[handle];

    uint32 openGLType = getOpenGLBufferType(ctx->bufferTypes[handle]);
    glBindBuffer(openGLType, id);
}

RENDERER_UPDATE_BUFFER(rendererUpdateBuffer)
{
    assert(handle < ctx->bufferCount);
    uint32 id = ctx->bufferIds[handle];

    uint32 openGLType = getOpenGLBufferType(ctx->bufferTypes[handle]);
    glBindBuffer(openGLType, id);
    glBufferData(openGLType, size, data, ctx->bufferUsages[handle]);
}

RENDERER_BIND_VERTEX_ATTRIBUTE(rendererBindVertexAttribute)
{
    glVertexAttribPointer(
        index, elementCount, elementType, isNormalized, stride, (void *)offset);
    glEnableVertexAttribArray(index);
    glVertexAttribDivisor(index, isPerInstance);
}

RENDERER_BIND_SHADER_STORAGE_BUFFER(rendererBindShaderStorageBuffer)
{
    assert(handle < ctx->bufferCount);
    uint32 id = ctx->bufferIds[handle];

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, id);
}

RENDERER_SET_VIEWPORT_SIZE(rendererSetViewportSize)
{
    glViewport(0, 0, width, height);
}

RENDERER_CLEAR_BACK_BUFFER(rendererClearBackBuffer)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

RENDERER_SET_POLYGON_MODE(rendererSetPolygonMode)
{
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
}

RENDERER_SET_BLEND_MODE(rendererSetBlendMode)
{
    if (enableDepthTest)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    glBlendEquation(equation);
    glBlendFunc(srcFactor, dstFactor);
}

RENDERER_DRAW_ELEMENTS(rendererDrawElements)
{
    glDrawElements(primitiveType, elementCount, GL_UNSIGNED_INT, 0);
}

RENDERER_DRAW_ELEMENTS_INSTANCED(rendererDrawElementsInstanced)
{
    glDrawElementsInstancedBaseInstance(
        primitiveType, elementCount, GL_UNSIGNED_INT, 0, instanceCount, instanceOffset);
}

RENDERER_DISPATCH_COMPUTE(rendererDispatchCompute)
{
    glDispatchCompute(xCount, yCount, zCount);
}

RENDERER_SHADER_STORAGE_MEMORY_BARRIER(rendererShaderStorageMemoryBarrier)
{
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
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
    assert(ctx->framebufferCount < RENDERER_MAX_FRAMEBUFFERS);
    uint32 *framebufferId = ctx->framebufferIds + ctx->framebufferCount;
    result->framebufferHandle = ctx->framebufferCount++;
    ctx->framebufferTextureIds[result->framebufferHandle] = result->textureId;
    glGenFramebuffers(1, framebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, *framebufferId);
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

    result->cameraTransform = glm::identity<glm::mat4>();
    result->clearColor = glm::vec4(0, 0, 0, 1);

    return result;
}

RENDERER_SET_CAMERA(rendererSetCamera)
{
    rq->cameraTransform = *transform;
}

RENDERER_CLEAR(rendererClear)
{
    rq->clearColor.r = r;
    rq->clearColor.g = g;
    rq->clearColor.b = b;
    rq->clearColor.a = a;
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

bool drawToTarget(RenderQueue *rq, uint32 width, uint32 height, RenderTarget *target)
{
    if (target)
    {
        uint32 framebufferId = rq->ctx->framebufferIds[target->framebufferHandle];
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
    }

    glViewport(0, 0, width, height);
    glClearColor(rq->clearColor.r, rq->clearColor.g, rq->clearColor.b, rq->clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GpuCameraState camera = {};
    camera.transform = rq->cameraTransform;
    glBindBuffer(GL_UNIFORM_BUFFER, rq->ctx->bufferIds[RENDERER_UNIFORM_BUFFER_CAMERA]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera), &camera);

    bool isMissingResources = false;

    glBindBuffer(GL_ARRAY_BUFFER, rq->ctx->quadInstanceBufferId);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(RenderQuad) * rq->quadCount, rq->ctx->quads, GL_STREAM_DRAW);

    RenderQueueCommandHeader *command = rq->firstCommand;
    while (command)
    {
        void *commandData = command + 1;

        switch (command->type)
        {
        case RENDER_CMD_DrawQuadsCommand:
        {
            DrawQuadsCommand *cmd = (DrawQuadsCommand *)commandData;
            RenderEffect *effect = cmd->effect;
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

                glBindVertexArray(rq->ctx->globalVertexArrayId);
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