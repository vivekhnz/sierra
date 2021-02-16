#ifndef TERRAIN_RENDERER_H
#define TERRAIN_RENDERER_H

#include <glm/gtc/type_ptr.hpp>

#include "terrain_platform.h"

enum RendererBufferType
{
    RENDERER_VERTEX_BUFFER,
    RENDERER_ELEMENT_BUFFER,
    RENDERER_SHADER_STORAGE_BUFFER
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

EXPORT uint32 rendererCreateTexture(EngineMemory *memory,
    uint32 elementType,
    uint32 cpuFormat,
    uint32 gpuFormat,
    uint32 width,
    uint32 height,
    uint32 wrapMode,
    uint32 filterMode);
EXPORT void rendererBindTexture(EngineMemory *memory, uint32 handle, uint8 slot);
EXPORT void rendererUpdateTexture(EngineMemory *memory,
    uint32 handle,
    uint32 elementType,
    uint32 cpuFormat,
    uint32 gpuFormat,
    uint32 width,
    uint32 height,
    void *pixels);
EXPORT void rendererReadTexturePixels(EngineMemory *memory,
    uint32 handle,
    uint32 elementType,
    uint32 gpuFormat,
    void *out_pixels);

EXPORT uint32 rendererCreateTextureArray(EngineMemory *memory,
    uint32 elementType,
    uint32 cpuFormat,
    uint32 gpuFormat,
    uint32 width,
    uint32 height,
    uint32 layers,
    uint32 wrapMode,
    uint32 filterMode);
EXPORT void rendererBindTextureArray(EngineMemory *memory, uint32 handle, uint8 slot);
EXPORT void rendererUpdateTextureArray(EngineMemory *memory,
    uint32 handle,
    uint32 elementType,
    uint32 gpuFormat,
    uint32 width,
    uint32 height,
    uint32 layer,
    void *pixels);

EXPORT uint32 rendererCreateFramebuffer(EngineMemory *memory, uint32 textureHandle);
EXPORT void rendererBindFramebuffer(EngineMemory *memory, uint32 handle);
EXPORT void rendererUnbindFramebuffer(EngineMemory *memory, uint32 handle);

EXPORT bool rendererCreateShader(
    EngineMemory *memory, uint32 type, char *src, uint32 *out_handle);

EXPORT bool rendererCreateShaderProgram(
    EngineMemory *memory, int shaderCount, uint32 *shaderHandles, uint32 *out_handle);
EXPORT void rendererUseShaderProgram(EngineMemory *memory, uint32 handle);
EXPORT void rendererSetShaderProgramUniformFloat(
    EngineMemory *memory, uint32 handle, const char *uniformName, float value);
EXPORT void rendererSetShaderProgramUniformInteger(
    EngineMemory *memory, uint32 handle, const char *uniformName, int32 value);
EXPORT void rendererSetShaderProgramUniformVector2(
    EngineMemory *memory, uint32 handle, const char *uniformName, glm::vec2 value);
EXPORT void rendererSetShaderProgramUniformVector3(
    EngineMemory *memory, uint32 handle, const char *uniformName, glm::vec3 value);
EXPORT void rendererSetShaderProgramUniformVector4(
    EngineMemory *memory, uint32 handle, const char *uniformName, glm::vec4 value);
EXPORT void rendererSetShaderProgramUniformMatrix4x4(
    EngineMemory *memory, uint32 handle, const char *uniformName, glm::mat4 value);

EXPORT uint32 rendererCreateVertexArray(EngineMemory *memory);
EXPORT void rendererBindVertexArray(EngineMemory *memory, uint32 handle);
EXPORT void rendererUnbindVertexArray();

EXPORT uint32 rendererCreateBuffer(
    EngineMemory *memory, RendererBufferType type, uint32 usage);
EXPORT void rendererBindBuffer(EngineMemory *memory, uint32 handle);
EXPORT void rendererUpdateBuffer(EngineMemory *memory, uint32 handle, uint64 size, void *data);

EXPORT void rendererBindVertexAttribute(uint8 index,
    uint32 elementType,
    bool isNormalized,
    uint8 elementCount,
    uint32 stride,
    uint32 offset,
    bool isPerInstance);
EXPORT void rendererBindShaderStorageBuffer(EngineMemory *memory, uint32 handle, uint8 slot);

EXPORT void rendererSetViewportSize(uint32 width, uint32 height);
EXPORT void rendererClearBackBuffer(float r, float g, float b, float a);
EXPORT void rendererSetPolygonMode(uint32 polygonMode);
EXPORT void rendererSetBlendMode(uint32 equation, uint32 srcFactor, uint32 dstFactor);
EXPORT void rendererDrawElementsInstanced(
    uint32 primitiveType, uint32 elementCount, uint32 instanceCount);

EXPORT void rendererDispatchCompute(uint32 xCount, uint32 yCount, uint32 zCount);
EXPORT void rendererShaderStorageMemoryBarrier();

EXPORT void rendererDestroyResources(EngineMemory *memory);

#endif