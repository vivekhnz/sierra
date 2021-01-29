#ifndef TERRAIN_RENDERER_H
#define TERRAIN_RENDERER_H

#include <glm/gtc/type_ptr.hpp>

#include "terrain_foundation.h"

enum RendererBufferType
{
    RENDERER_VERTEX_BUFFER,
    RENDERER_ELEMENT_BUFFER
};

EXPORT void rendererCreateUniformBuffers(MemoryBlock *memory);
EXPORT void rendererUpdateCameraState(MemoryBlock *memory, glm::mat4 *transform);
EXPORT void rendererUpdateLightingState(MemoryBlock *memory,
    glm::vec4 *lightDir,
    bool isLightingEnabled,
    bool isTextureEnabled,
    bool isNormalMapEnabled,
    bool isAOMapEnabled,
    bool isDisplacementMapEnabled);

EXPORT uint32 rendererCreateTexture(MemoryBlock *memory);
EXPORT uint32 rendererGetTextureId(MemoryBlock *memory, uint32 handle);
EXPORT void rendererBindTexture(MemoryBlock *memory, uint32 handle, uint8 slot);

EXPORT bool rendererCreateShader(
    MemoryBlock *memory, uint32 type, char *src, uint32 *out_handle);
EXPORT void rendererAttachShader(MemoryBlock *memory, uint32 shaderProgramId, uint32 handle);
EXPORT void rendererDetachShader(MemoryBlock *memory, uint32 shaderProgramId, uint32 handle);

EXPORT uint32 rendererCreateVertexArray(MemoryBlock *memory);
EXPORT void rendererBindVertexArray(MemoryBlock *memory, uint32 handle);
EXPORT void rendererUnbindVertexArray();

EXPORT uint32 rendererCreateBuffer(
    MemoryBlock *memory, RendererBufferType type, uint32 usage);
EXPORT void rendererBindBuffer(MemoryBlock *memory, uint32 handle);
EXPORT void rendererUpdateBuffer(MemoryBlock *memory, uint32 handle, uint64 size, void *data);

EXPORT void rendererBindVertexAttribute(uint8 index,
    uint32 elementType,
    bool isNormalized,
    uint8 elementCount,
    uint32 stride,
    uint32 offset,
    bool isPerInstance);
EXPORT void rendererBindShaderStorageBuffer(MemoryBlock *memory, uint32 handle, uint8 slot);

EXPORT void rendererSetViewportSize(uint32 width, uint32 height);
EXPORT void rendererClearBackBuffer(float r, float g, float b, float a);
EXPORT void rendererSetPolygonMode(uint32 polygonMode);
EXPORT void rendererSetBlendMode(uint32 equation, uint32 srcFactor, uint32 dstFactor);
EXPORT void rendererDrawElementsInstanced(
    uint32 primitiveType, uint32 elementCount, uint32 instanceCount);

EXPORT void rendererDestroyResources(MemoryBlock *memory);

#endif