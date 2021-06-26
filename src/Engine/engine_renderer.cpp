#include "engine_renderer.h"

extern EnginePlatformApi Platform;

#define RENDERER_MAX_TEXTURES 128
#define RENDERER_MAX_DEPTH_BUFFERS 128
#define RENDERER_MAX_FRAMEBUFFERS 128
#define RENDERER_MAX_SHADERS 128
#define RENDERER_MAX_SHADER_PROGRAMS 128
#define RENDERER_MAX_VERTEX_ARRAYS 128
#define RENDERER_MAX_BUFFERS 128

enum RendererUniformBuffer
{
    RENDERER_UNIFORM_BUFFER_CAMERA,
    RENDERER_UNIFORM_BUFFER_LIGHTING,

    RENDERER_UNIFORM_BUFFER_COUNT
};

struct RendererState
{
    uint32 textureCount;
    uint32 textureIds[RENDERER_MAX_TEXTURES];

    uint32 depthBufferCount;
    uint32 depthBufferIds[RENDERER_MAX_DEPTH_BUFFERS];

    uint32 framebufferCount;
    uint32 framebufferIds[RENDERER_MAX_FRAMEBUFFERS];
    uint32 framebufferTextureIds[RENDERER_MAX_FRAMEBUFFERS];

    uint32 shaderCount;
    uint32 shaderIds[RENDERER_MAX_SHADERS];

    uint32 shaderProgramCount;
    uint32 shaderProgramIds[RENDERER_MAX_SHADER_PROGRAMS];

    uint32 vertexArrayCount;
    uint32 vertexArrayIds[RENDERER_MAX_VERTEX_ARRAYS];

    uint32 bufferCount;
    uint32 bufferIds[RENDERER_MAX_BUFFERS];
    RendererBufferType bufferTypes[RENDERER_MAX_BUFFERS];
    uint32 bufferUsages[RENDERER_MAX_BUFFERS];
};

struct RenderQueue
{
    RendererState *state;

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

RendererState *getState(EngineMemory *memory)
{
    assert(memory->renderer.size >= sizeof(RendererState));
    RendererState *state = (RendererState *)memory->renderer.baseAddress;
    return state;
}

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
    RendererState *state = getState(memory);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    // note: we assume that this is called before any vertex or element buffers are created
    assert(state->bufferCount == 0);
    glGenBuffers(RENDERER_UNIFORM_BUFFER_COUNT, state->bufferIds);
    state->bufferCount = RENDERER_UNIFORM_BUFFER_COUNT;

    // initialize camera state
    uint32 cameraUboId = state->bufferIds[RENDERER_UNIFORM_BUFFER_CAMERA];
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

    uint32 lightingUboId = state->bufferIds[RENDERER_UNIFORM_BUFFER_LIGHTING];
    glBindBuffer(GL_UNIFORM_BUFFER, lightingUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(lighting), &lighting, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, RENDERER_UNIFORM_BUFFER_LIGHTING, lightingUboId, 0,
        sizeof(lighting));
}

RENDERER_UPDATE_CAMERA_STATE(rendererUpdateCameraState)
{
    RendererState *state = getState(memory);

    GpuCameraState camera;
    camera.transform = *transform;

    glBindBuffer(GL_UNIFORM_BUFFER, state->bufferIds[RENDERER_UNIFORM_BUFFER_CAMERA]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera), &camera);
}

RENDERER_UPDATE_LIGHTING_STATE(rendererUpdateLightingState)
{
    RendererState *state = getState(memory);

    GpuLightingState lighting;
    lighting.lightDir = *lightDir;
    lighting.isEnabled = isLightingEnabled;
    lighting.isTextureEnabled = isTextureEnabled;
    lighting.isNormalMapEnabled = isNormalMapEnabled;
    lighting.isAOMapEnabled = isAOMapEnabled;
    lighting.isDisplacementMapEnabled = isDisplacementMapEnabled;

    glBindBuffer(GL_UNIFORM_BUFFER, state->bufferIds[RENDERER_UNIFORM_BUFFER_LIGHTING]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lighting), &lighting);
}

RENDERER_CREATE_TEXTURE(rendererCreateTexture)
{
    RendererState *state = getState(memory);
    assert(state->textureCount < RENDERER_MAX_TEXTURES);

    uint32 *id = state->textureIds + state->textureCount;
    glGenTextures(1, id);

    glBindTexture(GL_TEXTURE_2D, *id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexImage2D(GL_TEXTURE_2D, 0, cpuFormat, width, height, 0, gpuFormat, elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    return state->textureCount++;
}

RENDERER_BIND_TEXTURE(rendererBindTexture)
{
    RendererState *state = getState(memory);
    assert(handle < state->textureCount);
    uint32 id = state->textureIds[handle];

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
}

RENDERER_UPDATE_TEXTURE(rendererUpdateTexture)
{
    RendererState *state = getState(memory);
    assert(handle < state->textureCount);
    uint32 id = state->textureIds[handle];

    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(
        GL_TEXTURE_2D, 0, cpuFormat, width, height, 0, gpuFormat, elementType, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
}

RENDERER_READ_TEXTURE_PIXELS(rendererReadTexturePixels)
{
    RendererState *state = getState(memory);
    assert(handle < state->textureCount);
    uint32 id = state->textureIds[handle];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, gpuFormat, elementType, out_pixels);
}

RENDERER_CREATE_TEXTURE_ARRAY(rendererCreateTextureArray)
{
    RendererState *state = getState(memory);
    assert(state->textureCount < RENDERER_MAX_TEXTURES);

    uint32 *id = state->textureIds + state->textureCount;
    glGenTextures(1, id);

    glBindTexture(GL_TEXTURE_2D_ARRAY, *id);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, cpuFormat, width, height, layers, 0, gpuFormat,
        elementType, 0);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    return state->textureCount++;
}

RENDERER_BIND_TEXTURE_ARRAY(rendererBindTextureArray)
{
    RendererState *state = getState(memory);
    assert(handle < state->textureCount);
    uint32 id = state->textureIds[handle];

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
}

RENDERER_UPDATE_TEXTURE_ARRAY(rendererUpdateTextureArray)
{
    RendererState *state = getState(memory);
    assert(handle < state->textureCount);
    uint32 id = state->textureIds[handle];

    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glTexSubImage3D(
        GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, width, height, 1, gpuFormat, elementType, pixels);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

RENDERER_CREATE_DEPTH_BUFFER(rendererCreateDepthBuffer)
{
    RendererState *state = getState(memory);
    assert(state->depthBufferCount < RENDERER_MAX_DEPTH_BUFFERS);

    uint32 *depthBufferId = state->depthBufferIds + state->depthBufferCount;
    glGenRenderbuffers(1, depthBufferId);

    glBindRenderbuffer(GL_RENDERBUFFER, *depthBufferId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    return state->depthBufferCount++;
}

RENDERER_RESIZE_DEPTH_BUFFER(rendererResizeDepthBuffer)
{
    RendererState *state = getState(memory);
    assert(handle < state->depthBufferCount);
    uint32 id = state->depthBufferIds[handle];

    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RENDERER_CREATE_FRAMEBUFFER(rendererCreateFramebuffer)
{
    RendererState *state = getState(memory);
    assert(state->framebufferCount < RENDERER_MAX_FRAMEBUFFERS);

    assert(textureHandle < state->textureCount);
    uint32 textureId = state->textureIds[textureHandle];

    uint32 *framebufferId = state->framebufferIds + state->framebufferCount;
    glGenFramebuffers(1, framebufferId);

    glBindFramebuffer(GL_FRAMEBUFFER, *framebufferId);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
    if (depthBufferHandle > -1)
    {
        uint32 depthBufferId = state->depthBufferIds[depthBufferHandle];
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferId);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    state->framebufferTextureIds[state->framebufferCount] = textureId;

    return state->framebufferCount++;
}

RENDERER_BIND_FRAMEBUFFER(rendererBindFramebuffer)
{
    RendererState *state = getState(memory);
    assert(handle < state->framebufferCount);
    uint32 id = state->framebufferIds[handle];

    glBindFramebuffer(GL_FRAMEBUFFER, id);
}

RENDERER_UNBIND_FRAMEBUFFER(rendererUnbindFramebuffer)
{
    RendererState *state = getState(memory);
    assert(handle < state->framebufferCount);

    uint32 textureId = state->framebufferTextureIds[handle];
    glBindTexture(GL_TEXTURE_2D, textureId);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RENDERER_CREATE_SHADER(rendererCreateShader)
{
    RendererState *state = getState(memory);
    assert(state->shaderCount < RENDERER_MAX_SHADERS);

    uint32 id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);

    glCompileShader(id);
    int32 succeeded;
    glGetShaderiv(id, GL_COMPILE_STATUS, &succeeded);
    if (succeeded)
    {
        state->shaderIds[state->shaderCount] = id;
        *out_handle = state->shaderCount++;

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
    RendererState *state = getState(memory);
    assert(state->shaderProgramCount < RENDERER_MAX_SHADER_PROGRAMS);

    uint32 id = glCreateProgram();
    for (int i = 0; i < shaderCount; i++)
    {
        glAttachShader(id, state->shaderIds[shaderHandles[i]]);
    }

    glLinkProgram(id);
    int32 succeeded;
    glGetProgramiv(id, GL_LINK_STATUS, &succeeded);
    if (succeeded)
    {
        for (int i = 0; i < shaderCount; i++)
        {
            glDetachShader(id, state->shaderIds[shaderHandles[i]]);
        }

        state->shaderProgramIds[state->shaderProgramCount] = id;
        *out_handle = state->shaderProgramCount++;
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
    RendererState *state = getState(memory);
    assert(handle < state->shaderProgramCount);
    uint32 id = state->shaderProgramIds[handle];

    glUseProgram(id);
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_FLOAT(rendererSetShaderProgramUniformFloat)
{
    RendererState *state = getState(memory);
    assert(handle < state->shaderProgramCount);
    uint32 id = state->shaderProgramIds[handle];

    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform1f(id, loc, value);
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_INTEGER(rendererSetShaderProgramUniformInteger)
{
    RendererState *state = getState(memory);
    assert(handle < state->shaderProgramCount);
    uint32 id = state->shaderProgramIds[handle];

    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform1i(id, loc, value);
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR2(rendererSetShaderProgramUniformVector2)
{
    RendererState *state = getState(memory);
    assert(handle < state->shaderProgramCount);
    uint32 id = state->shaderProgramIds[handle];

    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform2fv(id, loc, 1, glm::value_ptr(value));
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR3(rendererSetShaderProgramUniformVector3)
{
    RendererState *state = getState(memory);
    assert(handle < state->shaderProgramCount);
    uint32 id = state->shaderProgramIds[handle];

    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform3fv(id, loc, 1, glm::value_ptr(value));
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR4(rendererSetShaderProgramUniformVector4)
{
    RendererState *state = getState(memory);
    assert(handle < state->shaderProgramCount);
    uint32 id = state->shaderProgramIds[handle];

    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniform4fv(id, loc, 1, glm::value_ptr(value));
}

RENDERER_SET_SHADER_PROGRAM_UNIFORM_MATRIX4X4(rendererSetShaderProgramUniformMatrix4x4)
{
    RendererState *state = getState(memory);
    assert(handle < state->shaderProgramCount);
    uint32 id = state->shaderProgramIds[handle];

    uint32 loc = glGetUniformLocation(id, uniformName);
    glProgramUniformMatrix4fv(id, loc, 1, false, glm::value_ptr(value));
}

RENDERER_CREATE_VERTEX_ARRAY(rendererCreateVertexArray)
{
    RendererState *state = getState(memory);
    assert(state->vertexArrayCount < RENDERER_MAX_VERTEX_ARRAYS);
    glGenVertexArrays(1, state->vertexArrayIds + state->vertexArrayCount);
    return state->vertexArrayCount++;
}

RENDERER_BIND_VERTEX_ARRAY(rendererBindVertexArray)
{
    RendererState *state = getState(memory);
    assert(handle < state->vertexArrayCount);
    uint32 id = state->vertexArrayIds[handle];

    glBindVertexArray(id);
}

RENDERER_UNBIND_VERTEX_ARRAY(rendererUnbindVertexArray)
{
    glBindVertexArray(0);
}

RENDERER_CREATE_BUFFER(rendererCreateBuffer)
{
    RendererState *state = getState(memory);
    assert(state->bufferCount < RENDERER_MAX_BUFFERS);
    glGenBuffers(1, state->bufferIds + state->bufferCount);
    state->bufferTypes[state->bufferCount] = type;
    state->bufferUsages[state->bufferCount] = usage;
    return state->bufferCount++;
}

RENDERER_BIND_BUFFER(rendererBindBuffer)
{
    RendererState *state = getState(memory);
    assert(handle < state->bufferCount);
    uint32 id = state->bufferIds[handle];

    uint32 openGLType = getOpenGLBufferType(state->bufferTypes[handle]);
    glBindBuffer(openGLType, id);
}

RENDERER_UPDATE_BUFFER(rendererUpdateBuffer)
{
    RendererState *state = getState(memory);
    assert(handle < state->bufferCount);
    uint32 id = state->bufferIds[handle];

    uint32 openGLType = getOpenGLBufferType(state->bufferTypes[handle]);
    glBindBuffer(openGLType, id);
    glBufferData(openGLType, size, data, state->bufferUsages[handle]);
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
    RendererState *state = getState(memory);
    assert(handle < state->bufferCount);
    uint32 id = state->bufferIds[handle];

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

RENDERER_DESTROY_RESOURCES(rendererDestroyResources)
{
    RendererState *state = getState(memory);
    glDeleteTextures(state->textureCount, state->textureIds);
    glDeleteRenderbuffers(state->depthBufferCount, state->depthBufferIds);
    glDeleteFramebuffers(state->framebufferCount, state->framebufferIds);
    for (int i = 0; i < state->shaderCount; i++)
    {
        glDeleteShader(state->shaderIds[i]);
    }
    for (int i = 0; i < state->shaderProgramCount; i++)
    {
        glDeleteProgram(state->shaderProgramIds[i]);
    }
    glDeleteVertexArrays(state->vertexArrayCount, state->vertexArrayIds);
    glDeleteBuffers(state->bufferCount, state->bufferIds);
}

RENDERER_CREATE_QUEUE(rendererCreateQueue)
{
    assert(maxSize >= sizeof(RenderQueue));
    RenderQueue *result = (RenderQueue *)baseAddress;
    result->state = getState(memory);

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

    assert(shaderProgramHandle < rq->state->shaderProgramCount);
    rq->quad.shaderProgramId = rq->state->shaderProgramIds[shaderProgramHandle];

    assert(textureHandle < rq->state->textureCount);
    rq->quad.textureId = rq->state->textureIds[textureHandle];

    assert(vertexArrayHandle < rq->state->vertexArrayCount);
    rq->quad.vertexArrayId = rq->state->vertexArrayIds[vertexArrayHandle];
}

void drawToTarget(RenderQueue *rq, uint32 width, uint32 height, uint32 *framebufferHandle)
{
    uint32 framebufferId = 0;
    if (framebufferHandle)
    {
        assert(*framebufferHandle < rq->state->framebufferCount);
        framebufferId = rq->state->framebufferIds[*framebufferHandle];
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
    }

    glViewport(0, 0, width, height);
    glClearColor(rq->clearColor.r, rq->clearColor.g, rq->clearColor.b, rq->clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GpuCameraState camera = {};
    camera.transform = rq->cameraTransform;
    glBindBuffer(GL_UNIFORM_BUFFER, rq->state->bufferIds[RENDERER_UNIFORM_BUFFER_CAMERA]);
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
        uint32 framebufferTextureId = rq->state->framebufferTextureIds[*framebufferHandle];
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
