#ifndef ENGINE_RENDERER_H
#define ENGINE_RENDERER_H

#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

#include "engine_platform.h"

enum RendererBufferType
{
    RENDERER_VERTEX_BUFFER,
    RENDERER_ELEMENT_BUFFER,
    RENDERER_SHADER_STORAGE_BUFFER
};

#define RENDERER_MAX_TEXTURES 128
#define RENDERER_MAX_DEPTH_BUFFERS 128
#define RENDERER_MAX_FRAMEBUFFERS 128
#define RENDERER_MAX_SHADERS 128
#define RENDERER_MAX_SHADER_PROGRAMS 128
#define RENDERER_MAX_VERTEX_ARRAYS 128
#define RENDERER_MAX_BUFFERS 128
struct RenderContext
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

#define RENDERER_INITIALIZE(name) void name(RenderContext *ctx)
typedef RENDERER_INITIALIZE(RendererInitialize);

#define RENDERER_UPDATE_CAMERA_STATE(name) void name(RenderContext *ctx, glm::mat4 *transform)
typedef RENDERER_UPDATE_CAMERA_STATE(RendererUpdateCameraState);
#define RENDERER_UPDATE_LIGHTING_STATE(name)                                                  \
    void name(RenderContext *ctx, glm::vec4 *lightDir, bool isLightingEnabled,                \
        bool isTextureEnabled, bool isNormalMapEnabled, bool isAOMapEnabled,                  \
        bool isDisplacementMapEnabled)
typedef RENDERER_UPDATE_LIGHTING_STATE(RendererUpdateLightingState);

#define RENDERER_CREATE_TEXTURE(name)                                                         \
    uint32 name(RenderContext *ctx, uint32 elementType, uint32 cpuFormat, uint32 gpuFormat,   \
        uint32 width, uint32 height, uint32 wrapMode, uint32 filterMode)
typedef RENDERER_CREATE_TEXTURE(RendererCreateTexture);
#define RENDERER_BIND_TEXTURE(name) void name(RenderContext *ctx, uint32 handle, uint8 slot)
typedef RENDERER_BIND_TEXTURE(RendererBindTexture);
#define RENDERER_UPDATE_TEXTURE(name)                                                         \
    void name(RenderContext *ctx, uint32 handle, uint32 elementType, uint32 cpuFormat,        \
        uint32 gpuFormat, uint32 width, uint32 height, void *pixels)
typedef RENDERER_UPDATE_TEXTURE(RendererUpdateTexture);
#define RENDERER_READ_TEXTURE_PIXELS(name)                                                    \
    void name(RenderContext *ctx, uint32 handle, uint32 elementType, uint32 gpuFormat,        \
        void *out_pixels)
typedef RENDERER_READ_TEXTURE_PIXELS(RendererReadTexturePixels);

#define RENDERER_CREATE_TEXTURE_ARRAY(name)                                                   \
    uint32 name(RenderContext *ctx, uint32 elementType, uint32 cpuFormat, uint32 gpuFormat,   \
        uint32 width, uint32 height, uint32 layers, uint32 wrapMode, uint32 filterMode)
typedef RENDERER_CREATE_TEXTURE_ARRAY(RendererCreateTextureArray);
#define RENDERER_BIND_TEXTURE_ARRAY(name)                                                     \
    void name(RenderContext *ctx, uint32 handle, uint8 slot)
typedef RENDERER_BIND_TEXTURE_ARRAY(RendererBindTextureArray);
#define RENDERER_UPDATE_TEXTURE_ARRAY(name)                                                   \
    void name(RenderContext *ctx, uint32 handle, uint32 elementType, uint32 gpuFormat,        \
        uint32 width, uint32 height, uint32 layer, void *pixels)
typedef RENDERER_UPDATE_TEXTURE_ARRAY(RendererUpdateTextureArray);

#define RENDERER_CREATE_DEPTH_BUFFER(name)                                                    \
    uint32 name(RenderContext *ctx, uint32 width, uint32 height)
typedef RENDERER_CREATE_DEPTH_BUFFER(RendererCreateDepthBuffer);
#define RENDERER_RESIZE_DEPTH_BUFFER(name)                                                    \
    void name(RenderContext *ctx, uint32 handle, uint32 width, uint32 height)
typedef RENDERER_RESIZE_DEPTH_BUFFER(RendererResizeDepthBuffer);

#define RENDERER_CREATE_FRAMEBUFFER(name)                                                     \
    uint32 name(RenderContext *ctx, uint32 textureHandle, int32 depthBufferHandle)
typedef RENDERER_CREATE_FRAMEBUFFER(RendererCreateFramebuffer);
#define RENDERER_BIND_FRAMEBUFFER(name) void name(RenderContext *ctx, uint32 handle)
typedef RENDERER_BIND_FRAMEBUFFER(RendererBindFramebuffer);
#define RENDERER_UNBIND_FRAMEBUFFER(name) void name(RenderContext *ctx, uint32 handle)
typedef RENDERER_UNBIND_FRAMEBUFFER(RendererUnbindFramebuffer);

#define RENDERER_CREATE_SHADER(name)                                                          \
    bool name(RenderContext *ctx, uint32 type, char *src, uint32 *out_handle)
typedef RENDERER_CREATE_SHADER(RendererCreateShader);

#define RENDERER_CREATE_SHADER_PROGRAM(name)                                                  \
    bool name(RenderContext *ctx, int shaderCount, uint32 *shaderHandles, uint32 *out_handle)
typedef RENDERER_CREATE_SHADER_PROGRAM(RendererCreateShaderProgram);
#define RENDERER_USE_SHADER_PROGRAM(name) void name(RenderContext *ctx, uint32 handle)
typedef RENDERER_USE_SHADER_PROGRAM(RendererUseShaderProgram);
#define RENDERER_SET_SHADER_PROGRAM_UNIFORM_FLOAT(name)                                       \
    void name(RenderContext *ctx, uint32 handle, const char *uniformName, float value)
typedef RENDERER_SET_SHADER_PROGRAM_UNIFORM_FLOAT(RendererSetShaderProgramUniformFloat);
#define RENDERER_SET_SHADER_PROGRAM_UNIFORM_INTEGER(name)                                     \
    void name(RenderContext *ctx, uint32 handle, const char *uniformName, int32 value)
typedef RENDERER_SET_SHADER_PROGRAM_UNIFORM_INTEGER(RendererSetShaderProgramUniformInteger);
#define RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR2(name)                                     \
    void name(RenderContext *ctx, uint32 handle, const char *uniformName, glm::vec2 value)
typedef RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR2(RendererSetShaderProgramUniformVector2);
#define RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR3(name)                                     \
    void name(RenderContext *ctx, uint32 handle, const char *uniformName, glm::vec3 value)
typedef RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR3(RendererSetShaderProgramUniformVector3);
#define RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR4(name)                                     \
    void name(RenderContext *ctx, uint32 handle, const char *uniformName, glm::vec4 value)
typedef RENDERER_SET_SHADER_PROGRAM_UNIFORM_VECTOR4(RendererSetShaderProgramUniformVector4);
#define RENDERER_SET_SHADER_PROGRAM_UNIFORM_MATRIX4X4(name)                                   \
    void name(RenderContext *ctx, uint32 handle, const char *uniformName, glm::mat4 value)
typedef RENDERER_SET_SHADER_PROGRAM_UNIFORM_MATRIX4X4(
    RendererSetShaderProgramUniformMatrix4x4);

#define RENDERER_CREATE_VERTEX_ARRAY(name) uint32 name(RenderContext *ctx)
typedef RENDERER_CREATE_VERTEX_ARRAY(RendererCreateVertexArray);
#define RENDERER_BIND_VERTEX_ARRAY(name) void name(RenderContext *ctx, uint32 handle)
typedef RENDERER_BIND_VERTEX_ARRAY(RendererBindVertexArray);
#define RENDERER_UNBIND_VERTEX_ARRAY(name) void name()
typedef RENDERER_UNBIND_VERTEX_ARRAY(RendererUnbindVertexArray);

#define RENDERER_CREATE_BUFFER(name)                                                          \
    uint32 name(RenderContext *ctx, RendererBufferType type, uint32 usage)
typedef RENDERER_CREATE_BUFFER(RendererCreateBuffer);
#define RENDERER_BIND_BUFFER(name) void name(RenderContext *ctx, uint32 handle)
typedef RENDERER_BIND_BUFFER(RendererBindBuffer);
#define RENDERER_UPDATE_BUFFER(name)                                                          \
    void name(RenderContext *ctx, uint32 handle, uint64 size, void *data)
typedef RENDERER_UPDATE_BUFFER(RendererUpdateBuffer);

#define RENDERER_BIND_VERTEX_ATTRIBUTE(name)                                                  \
    void name(uint8 index, uint32 elementType, bool isNormalized, uint8 elementCount,         \
        uint32 stride, uint64 offset, bool isPerInstance)
typedef RENDERER_BIND_VERTEX_ATTRIBUTE(RendererBindVertexAttribute);
#define RENDERER_BIND_SHADER_STORAGE_BUFFER(name)                                             \
    void name(RenderContext *ctx, uint32 handle, uint8 slot)
typedef RENDERER_BIND_SHADER_STORAGE_BUFFER(RendererBindShaderStorageBuffer);

#define RENDERER_SET_VIEWPORT_SIZE(name) void name(uint32 width, uint32 height)
typedef RENDERER_SET_VIEWPORT_SIZE(RendererSetViewportSize);
#define RENDERER_CLEAR_BACK_BUFFER(name) void name(float r, float g, float b, float a)
typedef RENDERER_CLEAR_BACK_BUFFER(RendererClearBackBuffer);
#define RENDERER_SET_POLYGON_MODE(name) void name(uint32 polygonMode)
typedef RENDERER_SET_POLYGON_MODE(RendererSetPolygonMode);
#define RENDERER_SET_BLEND_MODE(name)                                                         \
    void name(uint32 equation, uint32 srcFactor, uint32 dstFactor, bool enableDepthTest)
typedef RENDERER_SET_BLEND_MODE(RendererSetBlendMode);
#define RENDERER_DRAW_ELEMENTS(name) void name(uint32 primitiveType, uint32 elementCount)
typedef RENDERER_DRAW_ELEMENTS(RendererDrawElements);
#define RENDERER_DRAW_ELEMENTS_INSTANCED(name)                                                \
    void name(uint32 primitiveType, uint32 elementCount, uint32 instanceCount,                \
        uint32 instanceOffset)
typedef RENDERER_DRAW_ELEMENTS_INSTANCED(RendererDrawElementsInstanced);

#define RENDERER_DISPATCH_COMPUTE(name) void name(uint32 xCount, uint32 yCount, uint32 zCount)
typedef RENDERER_DISPATCH_COMPUTE(RendererDispatchCompute);
#define RENDERER_SHADER_STORAGE_MEMORY_BARRIER(name) void name()
typedef RENDERER_SHADER_STORAGE_MEMORY_BARRIER(RendererShaderStorageMemoryBarrier);

struct RenderQueue;
#define RENDERER_CREATE_QUEUE(name) RenderQueue *name(RenderContext *ctx, MemoryArena *arena)
typedef RENDERER_CREATE_QUEUE(RendererCreateQueue);

#define RENDERER_SET_CAMERA(name) void name(RenderQueue *rq, glm::mat4 *transform)
typedef RENDERER_SET_CAMERA(RendererSetCamera);

#define RENDERER_CLEAR(name) void name(RenderQueue *rq, float r, float g, float b, float a)
typedef RENDERER_CLEAR(RendererClear);

#define RENDERER_PUSH_TEXTURED_QUAD(name)                                                     \
    void name(RenderQueue *rq, uint32 shaderProgramHandle, uint32 vertexArrayHandle,          \
        uint32 textureHandle)
typedef RENDERER_PUSH_TEXTURED_QUAD(RendererPushTexturedQuad);

#define RENDERER_DRAW_TO_TARGET(name)                                                         \
    void name(RenderQueue *rq, uint32 width, uint32 height, uint32 framebufferHandle)
typedef RENDERER_DRAW_TO_TARGET(RendererDrawToTarget);

#define RENDERER_DRAW_TO_SCREEN(name) void name(RenderQueue *rq, uint32 width, uint32 height)
typedef RENDERER_DRAW_TO_SCREEN(RendererDrawToScreen);

#endif