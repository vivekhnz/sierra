#include <glad/glad.h>

#include "terrain_renderer.h"

int rendererCreateTexture(EngineMemory *memory)
{
    assert(memory->size >= sizeof(RendererState));
    RendererState *state = (RendererState *)memory->address;

    assert(state->textureCount < RENDERER_MAX_TEXTURES);
    glGenTextures(1, state->textureIds + state->textureCount);
    return state->textureCount++;
}

unsigned int rendererGetTextureId(EngineMemory *memory, int handle)
{
    assert(memory->size >= sizeof(RendererState));
    RendererState *state = (RendererState *)memory->address;

    assert(handle < state->textureCount);
    return state->textureIds[handle];
}

void rendererBindTexture(EngineMemory *memory, int handle, int slot)
{
    assert(memory->size >= sizeof(RendererState));
    RendererState *state = (RendererState *)memory->address;

    assert(handle < state->textureCount);
    unsigned int id = state->textureIds[handle];

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
}

int rendererCreateVertexArray(EngineMemory *memory)
{
    assert(memory->size >= sizeof(RendererState));
    RendererState *state = (RendererState *)memory->address;

    assert(state->vertexArrayCount < RENDERER_MAX_VERTEX_ARRAYS);
    glGenVertexArrays(1, state->vertexArrayIds + state->vertexArrayCount);
    return state->vertexArrayCount++;
}

void rendererBindVertexArray(EngineMemory *memory, int handle)
{
    assert(memory->size >= sizeof(RendererState));
    RendererState *state = (RendererState *)memory->address;

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
    assert(memory->size >= sizeof(RendererState));
    RendererState *state = (RendererState *)memory->address;

    glDeleteTextures(state->textureCount, state->textureIds);
    glDeleteVertexArrays(state->vertexArrayCount, state->vertexArrayIds);
}