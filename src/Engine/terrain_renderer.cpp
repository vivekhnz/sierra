#include <glad/glad.h>

#include "terrain_renderer.h"

#define RENDERER_MAX_TEXTURES 128
#define RENDERER_MAX_VERTEX_ARRAYS 128

enum RendererUniformBuffer
{
    RENDERER_UNIFORM_BUFFER_CAMERA,
    RENDERER_UNIFORM_BUFFER_LIGHTING,

    RENDERER_UNIFORM_BUFFER_COUNT
};

struct RendererState
{
    unsigned int uniformBufferIds[RENDERER_UNIFORM_BUFFER_COUNT];

    int textureCount;
    unsigned int textureIds[RENDERER_MAX_TEXTURES];

    int vertexArrayCount;
    unsigned int vertexArrayIds[RENDERER_MAX_VERTEX_ARRAYS];
};

struct GpuCameraState
{
    glm::mat4 transform;
};
struct GpuLightingState
{
    glm::vec4 lightDir;
    int isEnabled;
    int isTextureEnabled;
    int isNormalMapEnabled;
    int isAOMapEnabled;
    int isDisplacementMapEnabled;
};

RendererState *getState(EngineMemory *memory)
{
    assert(memory->size >= sizeof(RendererState));
    RendererState *state = (RendererState *)memory->address;
    return state;
}

void rendererCreateUniformBuffers(EngineMemory *memory)
{
    RendererState *state = getState(memory);
    glGenBuffers(RENDERER_UNIFORM_BUFFER_COUNT, state->uniformBufferIds);

    // initialize camera state
    unsigned int cameraUboId = state->uniformBufferIds[RENDERER_UNIFORM_BUFFER_CAMERA];
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

    unsigned int lightingUboId = state->uniformBufferIds[RENDERER_UNIFORM_BUFFER_LIGHTING];
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

    glBindBuffer(GL_UNIFORM_BUFFER, state->uniformBufferIds[RENDERER_UNIFORM_BUFFER_CAMERA]);
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

    glBindBuffer(GL_UNIFORM_BUFFER, state->uniformBufferIds[RENDERER_UNIFORM_BUFFER_LIGHTING]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lighting), &lighting);
}

int rendererCreateTexture(EngineMemory *memory)
{
    RendererState *state = getState(memory);
    assert(state->textureCount < RENDERER_MAX_TEXTURES);
    glGenTextures(1, state->textureIds + state->textureCount);
    return state->textureCount++;
}

unsigned int rendererGetTextureId(EngineMemory *memory, int handle)
{
    RendererState *state = getState(memory);
    assert(handle < state->textureCount);
    return state->textureIds[handle];
}

void rendererBindTexture(EngineMemory *memory, int handle, int slot)
{
    RendererState *state = getState(memory);
    assert(handle < state->textureCount);
    unsigned int id = state->textureIds[handle];

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
}

int rendererCreateVertexArray(EngineMemory *memory)
{
    RendererState *state = getState(memory);
    assert(state->vertexArrayCount < RENDERER_MAX_VERTEX_ARRAYS);
    glGenVertexArrays(1, state->vertexArrayIds + state->vertexArrayCount);
    return state->vertexArrayCount++;
}

void rendererBindVertexArray(EngineMemory *memory, int handle)
{
    RendererState *state = getState(memory);
    assert(handle < state->vertexArrayCount);
    unsigned int id = state->vertexArrayIds[handle];

    glBindVertexArray(id);
}

void rendererSetViewportSize(int width, int height)
{
    glViewport(0, 0, width, height);
}

void rendererClearBackBuffer(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void rendererSetPolygonMode(unsigned int polygonMode)
{
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
}

void rendererSetBlendMode(
    unsigned int equation, unsigned int srcFactor, unsigned int dstFactor)
{
    glBlendEquation(equation);
    glBlendFunc(srcFactor, dstFactor);
}

void rendererDrawElementsInstanced(
    unsigned int primitiveType, int elementCount, int instanceCount)
{
    glDrawElementsInstanced(primitiveType, elementCount, GL_UNSIGNED_INT, 0, instanceCount);
}

void rendererDestroyResources(EngineMemory *memory)
{
    RendererState *state = getState(memory);
    glDeleteBuffers(RENDERER_UNIFORM_BUFFER_COUNT, state->uniformBufferIds);
    glDeleteTextures(state->textureCount, state->textureIds);
    glDeleteVertexArrays(state->vertexArrayCount, state->vertexArrayIds);
}