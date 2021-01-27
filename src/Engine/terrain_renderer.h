#ifndef TERRAIN_RENDERER_H
#define TERRAIN_RENDERER_H

#include <glm/gtc/type_ptr.hpp>

#include "terrain_foundation.h"

enum RendererBufferType
{
    RENDERER_VERTEX_BUFFER,
    RENDERER_ELEMENT_BUFFER
};

EXPORT void rendererCreateUniformBuffers(EngineMemory *memory);
EXPORT void rendererUpdateCameraState(EngineMemory *memory, glm::mat4 *transform);
EXPORT void rendererUpdateLightingState(EngineMemory *memory,
    glm::vec4 *lightDir,
    bool isLightingEnabled,
    bool isTextureEnabled,
    bool isNormalMapEnabled,
    bool isAOMapEnabled,
    bool isDisplacementMapEnabled);

EXPORT int rendererCreateTexture(EngineMemory *memory);
EXPORT unsigned int rendererGetTextureId(EngineMemory *memory, int handle);
EXPORT void rendererBindTexture(EngineMemory *memory, int handle, int slot);

EXPORT int rendererCreateVertexArray(EngineMemory *memory);
EXPORT void rendererBindVertexArray(EngineMemory *memory, int handle);
EXPORT void rendererUnbindVertexArray();

EXPORT int rendererCreateBuffer(
    EngineMemory *memory, RendererBufferType type, unsigned int usage);
EXPORT void rendererBindBuffer(EngineMemory *memory, int handle);
EXPORT void rendererUpdateBuffer(EngineMemory *memory, int handle, int size, void *data);

EXPORT void rendererBindVertexAttribute(unsigned int index,
    unsigned int elementType,
    bool isNormalized,
    unsigned int elementCount,
    unsigned int stride,
    unsigned int offset,
    bool isPerInstance);
EXPORT void rendererBindShaderStorageBuffer(EngineMemory *memory, int handle, int slot);

EXPORT void rendererSetViewportSize(int width, int height);
EXPORT void rendererClearBackBuffer(float r, float g, float b, float a);
EXPORT void rendererSetPolygonMode(unsigned int polygonMode);
EXPORT void rendererSetBlendMode(
    unsigned int equation, unsigned int srcFactor, unsigned int dstFactor);
EXPORT void rendererDrawElementsInstanced(
    unsigned int primitiveType, int elementCount, int instanceCount);

EXPORT void rendererDestroyResources(EngineMemory *memory);

#endif