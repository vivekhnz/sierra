#ifndef ENGINE_RENDERER_H
#define ENGINE_RENDERER_H

#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

enum ShaderType
{
    SHADER_TYPE_QUAD,
    SHADER_TYPE_MESH,
    SHADER_TYPE_TERRAIN
};

enum RendererBufferType
{
    RENDERER_VERTEX_BUFFER,
    RENDERER_ELEMENT_BUFFER,
    RENDERER_SHADER_STORAGE_BUFFER
};
struct RenderBuffer
{
    uint32 id;
    uint32 type;
    uint32 usage;
};

struct RenderMesh
{
    uint32 vertexBufferId;
    uint32 elementBufferId;
};

enum RenderTargetFormat
{
    RENDER_TARGET_FORMAT_RGB8_WITH_DEPTH,
    RENDER_TARGET_FORMAT_R16,
    RENDER_TARGET_FORMAT_R8UI_WITH_DEPTH,
    RENDER_TARGET_FORMAT_R16UI_WITH_DEPTH,
    RENDER_TARGET_FORMAT_R32UI_WITH_DEPTH
};

struct RenderTarget
{
    RenderTargetFormat format;
    uint32 width;
    uint32 height;
    uint32 textureId;
    uint32 depthTextureId;
    uint32 framebufferId;
};

enum RenderEffectBlendMode
{
    EFFECT_BLEND_ALPHA_BLEND,
    EFFECT_BLEND_ADDITIVE,
    EFFECT_BLEND_MAX
};

struct RenderQuad
{
    float x;
    float y;
    float width;
    float height;
};
struct RenderMeshInstance
{
    uint32 id;
    glm::mat4 transform;
};

struct RenderContext;
struct RenderQueue;
struct RenderEffect;

bool createShaderProgram(RenderContext *rctx, ShaderType type, char *src, uint32 *out_programId);
void destroyShaderProgram(uint32 id);

RenderMesh *createMesh(MemoryArena *arena, void *vertices, uint32 vertexCount, void *indices, uint32 indexCount);
void destroyMesh(RenderMesh *mesh);

#define RENDERER_INITIALIZE(name) RenderContext *name(MemoryArena *arena)
typedef RENDERER_INITIALIZE(RendererInitialize);

#define RENDERER_UPDATE_LIGHTING_STATE(name)                                                                      \
    void name(RenderContext *ctx, glm::vec4 *lightDir, bool isLightingEnabled, bool isTextureEnabled,             \
        bool isNormalMapEnabled, bool isAOMapEnabled, bool isDisplacementMapEnabled)
typedef RENDERER_UPDATE_LIGHTING_STATE(RendererUpdateLightingState);

#define RENDERER_CREATE_TEXTURE(name)                                                                             \
    uint32 name(uint32 elementType, uint32 cpuFormat, uint32 gpuFormat, uint32 width, uint32 height,              \
        uint32 wrapMode, uint32 filterMode)
typedef RENDERER_CREATE_TEXTURE(RendererCreateTexture);
#define RENDERER_UPDATE_TEXTURE(name)                                                                             \
    void name(uint32 id, uint32 elementType, uint32 cpuFormat, uint32 gpuFormat, uint32 width, uint32 height,     \
        void *pixels)
typedef RENDERER_UPDATE_TEXTURE(RendererUpdateTexture);
#define RENDERER_READ_TEXTURE_PIXELS(name)                                                                        \
    void name(uint32 id, uint32 elementType, uint32 gpuFormat, void *out_pixels)
typedef RENDERER_READ_TEXTURE_PIXELS(RendererReadTexturePixels);

#define RENDERER_CREATE_TEXTURE_ARRAY(name)                                                                       \
    uint32 name(uint32 elementType, uint32 cpuFormat, uint32 gpuFormat, uint32 width, uint32 height,              \
        uint32 layers, uint32 wrapMode, uint32 filterMode)
typedef RENDERER_CREATE_TEXTURE_ARRAY(RendererCreateTextureArray);
#define RENDERER_UPDATE_TEXTURE_ARRAY(name)                                                                       \
    void name(                                                                                                    \
        uint32 id, uint32 elementType, uint32 gpuFormat, uint32 width, uint32 height, uint32 layer, void *pixels)
typedef RENDERER_UPDATE_TEXTURE_ARRAY(RendererUpdateTextureArray);

#define RENDERER_CREATE_BUFFER(name) RenderBuffer name(RendererBufferType type, uint32 usage)
typedef RENDERER_CREATE_BUFFER(RendererCreateBuffer);
#define RENDERER_UPDATE_BUFFER(name) void name(RenderBuffer *buffer, uint64 size, void *data)
typedef RENDERER_UPDATE_BUFFER(RendererUpdateBuffer);

// render targets

#define RENDERER_CREATE_RENDER_TARGET(name)                                                                       \
    RenderTarget *name(MemoryArena *arena, uint32 width, uint32 height, RenderTargetFormat format)
typedef RENDERER_CREATE_RENDER_TARGET(RendererCreateRenderTarget);

#define RENDERER_RESIZE_RENDER_TARGET(name) void name(RenderTarget *target, uint32 width, uint32 height)
typedef RENDERER_RESIZE_RENDER_TARGET(RendererResizeRenderTarget);

#define RENDERER_GET_PIXELS(name)                                                                                 \
    void *name(MemoryArena *arena, RenderTarget *target, uint32 x, uint32 y, uint32 width, uint32 height,         \
        uint32 *out_pixelCount)
typedef RENDERER_GET_PIXELS(RendererGetPixels);

// effects

#define RENDERER_CREATE_EFFECT(name)                                                                              \
    RenderEffect *name(MemoryArena *arena, AssetHandle shader, RenderEffectBlendMode blendMode)
typedef RENDERER_CREATE_EFFECT(RendererCreateEffect);

#define RENDERER_SET_EFFECT_FLOAT(name) void name(RenderEffect *effect, char *paramName, float value)
typedef RENDERER_SET_EFFECT_FLOAT(RendererSetEffectFloat);

#define RENDERER_SET_EFFECT_INT(name) void name(RenderEffect *effect, char *paramName, int32 value)
typedef RENDERER_SET_EFFECT_INT(RendererSetEffectInt);

#define RENDERER_SET_EFFECT_UINT(name) void name(RenderEffect *effect, char *paramName, uint32 value)
typedef RENDERER_SET_EFFECT_UINT(RendererSetEffectUint);

#define RENDERER_SET_EFFECT_TEXTURE(name) void name(RenderEffect *effect, uint32 slot, uint32 textureId)
typedef RENDERER_SET_EFFECT_TEXTURE(RendererSetEffectTexture);

// render queue

#define RENDERER_CREATE_QUEUE(name) RenderQueue *name(RenderContext *ctx, MemoryArena *arena)
typedef RENDERER_CREATE_QUEUE(RendererCreateQueue);

#define RENDERER_SET_CAMERA_ORTHO(name) void name(RenderQueue *rq)
typedef RENDERER_SET_CAMERA_ORTHO(RendererSetCameraOrtho);
#define RENDERER_SET_CAMERA_PERSP(name)                                                                           \
    void name(RenderQueue *rq, glm::vec3 cameraPos, glm::vec3 lookAt, float fov)
typedef RENDERER_SET_CAMERA_PERSP(RendererSetCameraPersp);

#define RENDERER_CLEAR(name) void name(RenderQueue *rq, float r, float g, float b, float a)
typedef RENDERER_CLEAR(RendererClear);

#define RENDERER_PUSH_TEXTURED_QUAD(name)                                                                         \
    void name(RenderQueue *rq, RenderQuad quad, uint32 textureId, bool isTopDown)
typedef RENDERER_PUSH_TEXTURED_QUAD(RendererPushTexturedQuad);

#define RENDERER_PUSH_QUAD(name) void name(RenderQueue *rq, RenderQuad quad, RenderEffect *effect)
typedef RENDERER_PUSH_QUAD(RendererPushQuad);

#define RENDERER_PUSH_QUADS(name)                                                                                 \
    void name(RenderQueue *rq, RenderQuad *quads, uint32 quadCount, RenderEffect *effect)
typedef RENDERER_PUSH_QUADS(RendererPushQuads);

#define RENDERER_PUSH_MESHES(name)                                                                                \
    void name(RenderQueue *rq, AssetHandle mesh, RenderMeshInstance *instances, uint32 instanceCount,             \
        RenderEffect *effect)
typedef RENDERER_PUSH_MESHES(RendererPushMeshes);

#define RENDERER_PUSH_TERRAIN(name)                                                                               \
    void name(RenderQueue *rq, Heightfield *heightfield, AssetHandle terrainShader, uint32 heightmapTextureId,    \
        uint32 referenceHeightmapTextureId, uint32 meshVertexBufferId, uint32 meshElementBufferId,                \
        uint32 tessellationLevelBufferId, uint32 meshElementCount, uint32 materialCount,                          \
        uint32 albedoTextureArrayId, uint32 normalTextureArrayId, uint32 displacementTextureArrayId,              \
        uint32 aoTextureArrayId, uint32 materialPropsBufferId, bool isWireframe, uint32 visualizationMode,        \
        glm::vec2 cursorPos, float cursorRadius, float cursorFalloff)
typedef RENDERER_PUSH_TERRAIN(RendererPushTerrain);

#define RENDERER_DRAW_TO_TARGET(name) bool name(RenderQueue *rq, RenderTarget *target)
typedef RENDERER_DRAW_TO_TARGET(RendererDrawToTarget);

#define RENDERER_DRAW_TO_SCREEN(name) bool name(RenderQueue *rq, uint32 width, uint32 height)
typedef RENDERER_DRAW_TO_SCREEN(RendererDrawToScreen);

#endif