#ifndef TERRAIN_RENDERER_H
#define TERRAIN_RENDERER_H

#include "terrain_foundation.h"

#define RENDERER_MAX_TEXTURES 128
#define RENDERER_MAX_VERTEX_ARRAYS 128

struct RendererState
{
    int textureCount;
    unsigned int textureIds[RENDERER_MAX_TEXTURES];

    int vertexArrayCount;
    unsigned int vertexArrayIds[RENDERER_MAX_VERTEX_ARRAYS];
};

EXPORT int rendererCreateTexture(EngineMemory *memory);
EXPORT unsigned int rendererGetTextureId(EngineMemory *memory, int handle);
EXPORT void rendererBindTexture(EngineMemory *memory, int handle, int slot);

EXPORT int rendererCreateVertexArray(EngineMemory *memory);
EXPORT void rendererBindVertexArray(EngineMemory *memory, int handle);

EXPORT void rendererSetViewportSize(int width, int height);
EXPORT void rendererClearBackBuffer(float r, float g, float b, float a);
EXPORT void rendererSetPolygonMode(unsigned int polygonMode);
EXPORT void rendererSetBlendMode(
    unsigned int equation, unsigned int srcFactor, unsigned int dstFactor);
EXPORT void rendererDrawElementsInstanced(
    unsigned int primitiveType, int elementCount, int instanceCount);

EXPORT void rendererDestroyResources(EngineMemory *memory);

#endif