#ifndef ENGINE_RENDERER_H
#define ENGINE_RENDERER_H

struct RenderContext;
struct RenderEffect;
struct RenderQueue;

#define RENDERER_INITIALIZE(name) RenderContext *name(MemoryArena *arena)
typedef RENDERER_INITIALIZE(RendererInitialize);

// textures

#define RENDERER_CREATE_TEXTURE(name) TextureHandle name(uint32 width, uint32 height, TextureFormat format)
typedef RENDERER_CREATE_TEXTURE(RendererCreateTexture);
#define RENDERER_UPDATE_TEXTURE(name) void name(TextureHandle handle, uint32 width, uint32 height, void *pixels)
typedef RENDERER_UPDATE_TEXTURE(RendererUpdateTexture);
#define RENDERER_GET_PIXELS(name)                                                                                 \
    GetPixelsResult name(MemoryArena *arena, TextureHandle handle, uint32 width, uint32 height)
typedef RENDERER_GET_PIXELS(RendererGetPixels);
#define RENDERER_GET_PIXELS_IN_REGION(name)                                                                       \
    GetPixelsResult name(MemoryArena *arena, TextureHandle handle, uint32 x, uint32 y, uint32 width, uint32 height)
typedef RENDERER_GET_PIXELS_IN_REGION(RendererGetPixelsInRegion);

// texture arrays

#define RENDERER_GET_TEXTURE_ARRAY(name)                                                                          \
    TextureArrayHandle name(RenderContext *ctx, uint32 width, uint32 height, TextureFormat format)
typedef RENDERER_GET_TEXTURE_ARRAY(RendererGetTextureArray);
#define RENDERER_RESERVE_TEXTURE_SLOT(name) uint16 name(TextureArrayHandle handle)
typedef RENDERER_RESERVE_TEXTURE_SLOT(RendererReserveTextureSlot);
#define RENDERER_UPDATE_TEXTURE_ARRAY(name) void name(TextureArrayHandle handle, uint32 layer, void *pixels)
typedef RENDERER_UPDATE_TEXTURE_ARRAY(RendererUpdateTextureArray);

// render targets

#define RENDERER_CREATE_RENDER_TARGET(name)                                                                       \
    RenderTarget *name(                                                                                           \
        MemoryArena *arena, uint32 width, uint32 height, TextureFormat format, bool createDepthBuffer)
typedef RENDERER_CREATE_RENDER_TARGET(RendererCreateRenderTarget);

#define RENDERER_RESIZE_RENDER_TARGET(name) void name(RenderTarget *target, uint32 width, uint32 height)
typedef RENDERER_RESIZE_RENDER_TARGET(RendererResizeRenderTarget);

#define getBounds(renderTarget)                                                                                   \
    {                                                                                                             \
        0, 0, (float)(renderTarget->width), (float)(renderTarget->height)                                         \
    }

// effects

#define RENDERER_CREATE_EFFECT(name)                                                                              \
    RenderEffect *name(MemoryArena *arena, AssetHandle shaderAsset, RenderEffectBlendMode blendMode)
typedef RENDERER_CREATE_EFFECT(RendererCreateEffect);

#define RENDERER_SET_EFFECT_FLOAT(name) void name(RenderEffect *effect, char *paramName, float value)
typedef RENDERER_SET_EFFECT_FLOAT(RendererSetEffectFloat);

#define RENDERER_SET_EFFECT_VEC3(name) void name(RenderEffect *effect, char *paramName, glm::vec3 value)
typedef RENDERER_SET_EFFECT_VEC3(RendererSetEffectVec3);

#define RENDERER_SET_EFFECT_INT(name) void name(RenderEffect *effect, char *paramName, int32 value)
typedef RENDERER_SET_EFFECT_INT(RendererSetEffectInt);

#define RENDERER_SET_EFFECT_UINT(name) void name(RenderEffect *effect, char *paramName, uint32 value)
typedef RENDERER_SET_EFFECT_UINT(RendererSetEffectUint);

#define RENDERER_SET_EFFECT_TEXTURE(name) void name(RenderEffect *effect, uint32 slot, TextureHandle handle)
typedef RENDERER_SET_EFFECT_TEXTURE(RendererSetEffectTexture);

// render queue

#define RENDERER_CREATE_QUEUE(name) RenderQueue *name(RenderContext *ctx, MemoryArena *arena)
typedef RENDERER_CREATE_QUEUE(RendererCreateQueue);

#define RENDERER_SET_CAMERA_ORTHO(name) void name(RenderQueue *rq)
typedef RENDERER_SET_CAMERA_ORTHO(RendererSetCameraOrtho);
#define RENDERER_SET_CAMERA_ORTHO_OFFSET(name) void name(RenderQueue *rq, glm::vec2 cameraPos)
typedef RENDERER_SET_CAMERA_ORTHO_OFFSET(RendererSetCameraOrthoOffset);
#define RENDERER_SET_CAMERA_PERSP(name)                                                                           \
    void name(RenderQueue *rq, glm::vec3 cameraPos, glm::vec3 lookAt, float fov)
typedef RENDERER_SET_CAMERA_PERSP(RendererSetCameraPersp);

#define RENDERER_SET_LIGHTING(name)                                                                               \
    void name(RenderQueue *rq, glm::vec4 *lightDir, bool isLightingEnabled, bool isTextureEnabled,                \
        bool isNormalMapEnabled, bool isAOMapEnabled, bool isDisplacementMapEnabled)
typedef RENDERER_SET_LIGHTING(RendererSetLighting);

#define RENDERER_CLEAR(name) void name(RenderQueue *rq, float r, float g, float b, float a)
typedef RENDERER_CLEAR(RendererClear);

#define RENDERER_PUSH_TEXTURED_QUAD(name)                                                                         \
    void name(RenderQueue *rq, RenderQuad quad, TextureHandle textureHandle, bool isTopDown)
typedef RENDERER_PUSH_TEXTURED_QUAD(RendererPushTexturedQuad);

#define RENDERER_PUSH_COLORED_QUAD(name) void name(RenderQueue *rq, RenderQuad quad, glm::vec3 color)
typedef RENDERER_PUSH_COLORED_QUAD(RendererPushColoredQuad);

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
    void name(RenderQueue *rq, Heightfield *heightfield, glm::vec2 heightmapSize, AssetHandle terrainShader,      \
        TextureHandle heightmapTexture, TextureHandle referenceHeightmapTexture,                                  \
        TextureHandle xAdjacentHeightmapTexture, TextureHandle xAdjacentReferenceHeightmapTexture,                \
        TextureHandle yAdjacentHeightmapTexture, TextureHandle yAdjacentReferenceHeightmapTexture,                \
        TextureHandle oppositeHeightmapTexture, TextureHandle oppositeReferenceHeightmapTexture,                  \
        uint32 materialCount, TextureArrayHandle textureArray_RGBA8_2048x2048,                                    \
        TextureArrayHandle textureArray_R16_2048x2048, TextureArrayHandle textureArray_R8_2048x2048,              \
        RenderTerrainMaterial *materials, bool isWireframe, uint32 visualizationMode, glm::vec2 cursorPos,        \
        float cursorRadius, float cursorFalloff)
typedef RENDERER_PUSH_TERRAIN(RendererPushTerrain);

#define RENDERER_DRAW_TO_TARGET(name) bool name(RenderQueue *rq, RenderTarget *target)
typedef RENDERER_DRAW_TO_TARGET(RendererDrawToTarget);

#define RENDERER_DRAW_TO_SCREEN(name) bool name(RenderQueue *rq, uint32 width, uint32 height)
typedef RENDERER_DRAW_TO_SCREEN(RendererDrawToScreen);

#endif