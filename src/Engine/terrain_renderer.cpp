#include <glad/glad.h>

#include "terrain_renderer.h"

#define RENDERER_MAX_TEXTURES 128
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

    uint32 vertexArrayCount;
    uint32 vertexArrayIds[RENDERER_MAX_VERTEX_ARRAYS];

    uint32 bufferCount;
    uint32 bufferIds[RENDERER_MAX_BUFFERS];
    RendererBufferType bufferTypes[RENDERER_MAX_BUFFERS];
    uint32 bufferUsages[RENDERER_MAX_BUFFERS];
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
    assert(memory->size >= sizeof(RendererState));
    RendererState *state = (RendererState *)memory->address;
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
    assert(bufferType);
    return bufferType;
}

void rendererCreateUniformBuffers(EngineMemory *memory)
{
    RendererState *state = getState(memory);

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

void rendererUpdateCameraState(EngineMemory *memory, glm::mat4 *transform)
{
    RendererState *state = getState(memory);

    GpuCameraState camera;
    camera.transform = *transform;

    glBindBuffer(GL_UNIFORM_BUFFER, state->bufferIds[RENDERER_UNIFORM_BUFFER_CAMERA]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera), &camera);
}

void rendererUpdateLightingState(EngineMemory *memory,
    glm::vec4 *lightDir,
    bool isLightingEnabled,
    bool isTextureEnabled,
    bool isNormalMapEnabled,
    bool isAOMapEnabled,
    bool isDisplacementMapEnabled)
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

uint32 rendererCreateTexture(EngineMemory *memory)
{
    RendererState *state = getState(memory);
    assert(state->textureCount < RENDERER_MAX_TEXTURES);
    glGenTextures(1, state->textureIds + state->textureCount);
    return state->textureCount++;
}

uint32 rendererGetTextureId(EngineMemory *memory, uint32 handle)
{
    RendererState *state = getState(memory);
    assert(handle < state->textureCount);
    return state->textureIds[handle];
}

void rendererBindTexture(EngineMemory *memory, uint32 handle, uint8 slot)
{
    RendererState *state = getState(memory);
    assert(handle < state->textureCount);
    uint32 id = state->textureIds[handle];

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
}

uint32 rendererCreateVertexArray(EngineMemory *memory)
{
    RendererState *state = getState(memory);
    assert(state->vertexArrayCount < RENDERER_MAX_VERTEX_ARRAYS);
    glGenVertexArrays(1, state->vertexArrayIds + state->vertexArrayCount);
    return state->vertexArrayCount++;
}

void rendererBindVertexArray(EngineMemory *memory, uint32 handle)
{
    RendererState *state = getState(memory);
    assert(handle < state->vertexArrayCount);
    uint32 id = state->vertexArrayIds[handle];

    glBindVertexArray(id);
}

void rendererUnbindVertexArray()
{
    glBindVertexArray(0);
}

uint32 rendererCreateBuffer(EngineMemory *memory, RendererBufferType type, uint32 usage)
{
    RendererState *state = getState(memory);
    assert(state->bufferCount < RENDERER_MAX_BUFFERS);
    glGenBuffers(1, state->bufferIds + state->bufferCount);
    state->bufferTypes[state->bufferCount] = type;
    state->bufferUsages[state->bufferCount] = usage;
    return state->bufferCount++;
}

void rendererBindBuffer(EngineMemory *memory, uint32 handle)
{
    RendererState *state = getState(memory);
    assert(handle < state->bufferCount);
    uint32 id = state->bufferIds[handle];

    uint32 openGLType = getOpenGLBufferType(state->bufferTypes[handle]);
    glBindBuffer(openGLType, id);
}

void rendererUpdateBuffer(EngineMemory *memory, uint32 handle, uint64 size, void *data)
{
    RendererState *state = getState(memory);
    assert(handle < state->bufferCount);
    uint32 id = state->bufferIds[handle];

    uint32 openGLType = getOpenGLBufferType(state->bufferTypes[handle]);
    glBindBuffer(openGLType, id);
    glBufferData(openGLType, size, data, state->bufferUsages[handle]);
}

void rendererBindVertexAttribute(uint8 index,
    uint32 elementType,
    bool isNormalized,
    uint8 elementCount,
    uint32 stride,
    uint32 offset,
    bool isPerInstance)
{
    glVertexAttribPointer(
        index, elementCount, elementType, isNormalized, stride, (void *)offset);
    glEnableVertexAttribArray(index);
    glVertexAttribDivisor(index, isPerInstance);
}

void rendererBindShaderStorageBuffer(EngineMemory *memory, uint32 handle, uint8 slot)
{
    RendererState *state = getState(memory);
    assert(handle < state->bufferCount);
    uint32 id = state->bufferIds[handle];

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, id);
}

void rendererSetViewportSize(uint32 width, uint32 height)
{
    glViewport(0, 0, width, height);
}

void rendererClearBackBuffer(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void rendererSetPolygonMode(uint32 polygonMode)
{
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
}

void rendererSetBlendMode(uint32 equation, uint32 srcFactor, uint32 dstFactor)
{
    glBlendEquation(equation);
    glBlendFunc(srcFactor, dstFactor);
}

void rendererDrawElementsInstanced(
    uint32 primitiveType, uint32 elementCount, uint32 instanceCount)
{
    glDrawElementsInstanced(primitiveType, elementCount, GL_UNSIGNED_INT, 0, instanceCount);
}

void rendererDestroyResources(EngineMemory *memory)
{
    RendererState *state = getState(memory);
    glDeleteTextures(state->textureCount, state->textureIds);
    glDeleteVertexArrays(state->vertexArrayCount, state->vertexArrayIds);
    glDeleteBuffers(state->bufferCount, state->bufferIds);
}