#include "engine_renderer.h"

extern EnginePlatformApi Platform;

enum RendererUniformBuffer
{
    RENDERER_UNIFORM_BUFFER_CAMERA,
    RENDERER_UNIFORM_BUFFER_LIGHTING,

    RENDERER_UNIFORM_BUFFER_COUNT
};

struct RenderQueue
{
    MemoryArena *arena;
    RenderContext *ctx;

    glm::mat4 cameraTransform;
    glm::vec4 clearColor;

    struct
    {
        bool render;
        uint32 shaderProgramId;
        uint32 textureId;
        uint32 vertexArrayId;
    } quad;
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
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    // note: we assume that this is called before any vertex or element buffers are created
    assert(ctx->bufferCount == 0);
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
    assert(ctx->textureCount < RENDERER_MAX_TEXTURES);

    uint32 *id = ctx->textureIds + ctx->textureCount;
    glGenTextures(1, id);

    glBindTexture(GL_TEXTURE_2D, *id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexImage2D(GL_TEXTURE_2D, 0, cpuFormat, width, height, 0, gpuFormat, elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    return ctx->textureCount++;
}

RENDERER_BIND_TEXTURE(rendererBindTexture)
{
    assert(handle < ctx->textureCount);
    uint32 id = ctx->textureIds[handle];

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
}

RENDERER_UPDATE_TEXTURE(rendererUpdateTexture)
{
    assert(handle < ctx->textureCount);
    uint32 id = ctx->textureIds[handle];

    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(
        GL_TEXTURE_2D, 0, cpuFormat, width, height, 0, gpuFormat, elementType, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
}

RENDERER_READ_TEXTURE_PIXELS(rendererReadTexturePixels)
{
    assert(handle < ctx->textureCount);
    uint32 id = ctx->textureIds[handle];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, gpuFormat, elementType, out_pixels);
}

RENDERER_CREATE_TEXTURE_ARRAY(rendererCreateTextureArray)
{
    assert(ctx->textureCount < RENDERER_MAX_TEXTURES);

    uint32 *id = ctx->textureIds + ctx->textureCount;
    glGenTextures(1, id);

    glBindTexture(GL_TEXTURE_2D_ARRAY, *id);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, cpuFormat, width, height, layers, 0, gpuFormat,
        elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    return ctx->textureCount++;
}

RENDERER_BIND_TEXTURE_ARRAY(rendererBindTextureArray)
{
    assert(handle < ctx->textureCount);
    uint32 id = ctx->textureIds[handle];

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
}

RENDERER_UPDATE_TEXTURE_ARRAY(rendererUpdateTextureArray)
{
    assert(handle < ctx->textureCount);
    uint32 id = ctx->textureIds[handle];

    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glTexSubImage3D(
        GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, width, height, 1, gpuFormat, elementType, pixels);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

RENDERER_CREATE_DEPTH_BUFFER(rendererCreateDepthBuffer)
{
    assert(ctx->depthBufferCount < RENDERER_MAX_DEPTH_BUFFERS);

    uint32 *depthBufferId = ctx->depthBufferIds + ctx->depthBufferCount;
    glGenRenderbuffers(1, depthBufferId);

    glBindRenderbuffer(GL_RENDERBUFFER, *depthBufferId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    return ctx->depthBufferCount++;
}

RENDERER_RESIZE_DEPTH_BUFFER(rendererResizeDepthBuffer)
{
    assert(handle < ctx->depthBufferCount);
    uint32 id = ctx->depthBufferIds[handle];

    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RENDERER_CREATE_FRAMEBUFFER(rendererCreateFramebuffer)
{
    assert(ctx->framebufferCount < RENDERER_MAX_FRAMEBUFFERS);

    assert(textureHandle < ctx->textureCount);
    uint32 textureId = ctx->textureIds[textureHandle];

    uint32 *framebufferId = ctx->framebufferIds + ctx->framebufferCount;
    glGenFramebuffers(1, framebufferId);

    glBindFramebuffer(GL_FRAMEBUFFER, *framebufferId);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
    if (depthBufferHandle > -1)
    {
        uint32 depthBufferId = ctx->depthBufferIds[depthBufferHandle];
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferId);
    }

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
    assert(ctx->shaderCount < RENDERER_MAX_SHADERS);

    uint32 id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);

    glCompileShader(id);
    int32 succeeded;
    glGetShaderiv(id, GL_COMPILE_STATUS, &succeeded);
    if (succeeded)
    {
        ctx->shaderIds[ctx->shaderCount] = id;
        *out_handle = ctx->shaderCount++;

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
    assert(ctx->shaderProgramCount < RENDERER_MAX_SHADER_PROGRAMS);

    uint32 id = glCreateProgram();
    for (int i = 0; i < shaderCount; i++)
    {
        glAttachShader(id, ctx->shaderIds[shaderHandles[i]]);
    }

    glLinkProgram(id);
    int32 succeeded;
    glGetProgramiv(id, GL_LINK_STATUS, &succeeded);
    if (succeeded)
    {
        for (int i = 0; i < shaderCount; i++)
        {
            glDetachShader(id, ctx->shaderIds[shaderHandles[i]]);
        }

        ctx->shaderProgramIds[ctx->shaderProgramCount] = id;
        *out_handle = ctx->shaderProgramCount++;
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
    assert(handle < ctx->shaderProgramCount);
    uint32 id = ctx->shaderProgramIds[handle];

    glUseProgram(id);
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_FLOAT(rendererSetShaderProgramUniformFloat)
{
    assert(handle < ctx->shaderProgramCount);
    uint32 id = ctx->shaderProgramIds[handle];

    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform1f(id, loc, value);
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_INTEGER(rendererSetShaderProgramUniformInteger)
{
    assert(handle < ctx->shaderProgramCount);
    uint32 id = ctx->shaderProgramIds[handle];

    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform1i(id, loc, value);
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR2(rendererSetShaderProgramUniformVector2)
{
    assert(handle < ctx->shaderProgramCount);
    uint32 id = ctx->shaderProgramIds[handle];

    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform2fv(id, loc, 1, glm::value_ptr(value));
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR3(rendererSetShaderProgramUniformVector3)
{
    assert(handle < ctx->shaderProgramCount);
    uint32 id = ctx->shaderProgramIds[handle];

    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform3fv(id, loc, 1, glm::value_ptr(value));
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR4(rendererSetShaderProgramUniformVector4)
{
    assert(handle < ctx->shaderProgramCount);
    uint32 id = ctx->shaderProgramIds[handle];

    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform4fv(id, loc, 1, glm::value_ptr(value));
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_MATRIX4X4(rendererSetShaderProgramUniformMatrix4x4)
{
    assert(handle < ctx->shaderProgramCount);
    uint32 id = ctx->shaderProgramIds[handle];

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

RENDERER_CREATE_QUEUE(rendererCreateQueue)
{
    RenderQueue *result = pushStruct(arena, RenderQueue);
    result->arena = arena;
    result->ctx = ctx;

    result->cameraTransform = glm::identity<glm::mat4>();
    result->clearColor = glm::vec4(0, 0, 0, 1);
    result->quad.render = false;

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

RENDERER_PUSH_TEXTURED_QUAD(rendererPushTexturedQuad)
{
    rq->quad.render = true;

    assert(shaderProgramHandle < rq->ctx->shaderProgramCount);
    rq->quad.shaderProgramId = rq->ctx->shaderProgramIds[shaderProgramHandle];

    assert(textureHandle < rq->ctx->textureCount);
    rq->quad.textureId = rq->ctx->textureIds[textureHandle];

    assert(vertexArrayHandle < rq->ctx->vertexArrayCount);
    rq->quad.vertexArrayId = rq->ctx->vertexArrayIds[vertexArrayHandle];
}

void drawToTarget(RenderQueue *rq, uint32 width, uint32 height, uint32 *framebufferHandle)
{
    uint32 framebufferId = 0;
    if (framebufferHandle)
    {
        assert(*framebufferHandle < rq->ctx->framebufferCount);
        framebufferId = rq->ctx->framebufferIds[*framebufferHandle];
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
    }

    glViewport(0, 0, width, height);
    glClearColor(rq->clearColor.r, rq->clearColor.g, rq->clearColor.b, rq->clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GpuCameraState camera = {};
    camera.transform = rq->cameraTransform;
    glBindBuffer(GL_UNIFORM_BUFFER, rq->ctx->bufferIds[RENDERER_UNIFORM_BUFFER_CAMERA]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera), &camera);

    if (rq->quad.render)
    {
        glUseProgram(rq->quad.shaderProgramId);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rq->quad.textureId);
        glBindVertexArray(rq->quad.vertexArrayId);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    if (framebufferHandle)
    {
        uint32 framebufferTextureId = rq->ctx->framebufferTextureIds[*framebufferHandle];
        glBindTexture(GL_TEXTURE_2D, framebufferTextureId);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

RENDERER_DRAW_TO_TARGET(rendererDrawToTarget)
{
    drawToTarget(rq, width, height, &framebufferHandle);
}
RENDERER_DRAW_TO_SCREEN(rendererDrawToScreen)
{
    drawToTarget(rq, width, height, 0);
}