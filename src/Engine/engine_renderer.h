#ifndef ENGINE_RENDERER_H
#define ENGINE_RENDERER_H

struct RenderContext;
struct RenderEffect;
struct RenderQueue;

#define RENDERER_INITIALIZE(name) RenderContext *name(MemoryArena *arena)

// textures

#define RENDERER_CREATE_TEXTURE(name) TextureHandle name(uint32 width, uint32 height, TextureFormat format)
#define RENDERER_UPDATE_TEXTURE(name) void name(TextureHandle handle, uint32 width, uint32 height, void *pixels)
#define RENDERER_GET_PIXELS(name)                                                                                 \
    GetPixelsResult name(MemoryArena *arena, TextureHandle handle, uint32 width, uint32 height)
#define RENDERER_GET_PIXELS_IN_REGION(name)                                                                       \
    GetPixelsResult name(MemoryArena *arena, TextureHandle handle, uint32 x, uint32 y, uint32 width, uint32 height)

// render targets

#define RENDERER_CREATE_RENDER_TARGET(name)                                                                       \
    RenderTarget *name(                                                                                           \
        MemoryArena *arena, uint32 width, uint32 height, TextureFormat format, bool createDepthBuffer)
#define RENDERER_RESIZE_RENDER_TARGET(name) void name(RenderTarget *target, uint32 width, uint32 height)
#define getBounds(renderTarget)                                                                                   \
    {                                                                                                             \
        0, 0, (float)(renderTarget->width), (float)(renderTarget->height)                                         \
    }

// effects

#define RENDERER_CREATE_EFFECT(name)                                                                              \
    RenderEffect *name(MemoryArena *arena, AssetHandle shaderAsset, RenderEffectBlendMode blendMode)
#define RENDERER_SET_EFFECT_FLOAT(name) void name(RenderEffect *effect, char *paramName, float value)
#define RENDERER_SET_EFFECT_VEC3(name) void name(RenderEffect *effect, char *paramName, glm::vec3 value)
#define RENDERER_SET_EFFECT_INT(name) void name(RenderEffect *effect, char *paramName, int32 value)
#define RENDERER_SET_EFFECT_UINT(name) void name(RenderEffect *effect, char *paramName, uint32 value)
#define RENDERER_SET_EFFECT_TEXTURE(name) void name(RenderEffect *effect, uint32 slot, TextureHandle handle)

// render queue

#define RENDERER_CREATE_QUEUE(name) RenderQueue *name(RenderContext *ctx, MemoryArena *arena)
#define RENDERER_SET_CAMERA_ORTHO(name) void name(RenderQueue *rq)
#define RENDERER_SET_CAMERA_ORTHO_OFFSET(name) void name(RenderQueue *rq, glm::vec2 cameraPos)
#define RENDERER_SET_CAMERA_PERSP(name)                                                                           \
    void name(RenderQueue *rq, glm::vec3 cameraPos, glm::vec3 lookAt, float fov)
#define RENDERER_SET_LIGHTING(name)                                                                               \
    void name(RenderQueue *rq, glm::vec4 *lightDir, bool isLightingEnabled, bool isTextureEnabled,                \
        bool isNormalMapEnabled, bool isAOMapEnabled, bool isDisplacementMapEnabled)
#define RENDERER_CLEAR(name) void name(RenderQueue *rq, float r, float g, float b, float a)
#define RENDERER_PUSH_TEXTURED_QUAD(name)                                                                         \
    void name(RenderQueue *rq, RenderQuad quad, TextureHandle textureHandle, bool isTopDown)
#define RENDERER_PUSH_COLORED_QUAD(name) void name(RenderQueue *rq, RenderQuad quad, glm::vec3 color)
#define RENDERER_PUSH_QUAD(name) void name(RenderQueue *rq, RenderQuad quad, RenderEffect *effect)
#define RENDERER_PUSH_QUADS(name)                                                                                 \
    void name(RenderQueue *rq, RenderQuad *quads, uint32 quadCount, RenderEffect *effect)
#define RENDERER_PUSH_MESHES(name)                                                                                \
    void name(RenderQueue *rq, AssetHandle mesh, RenderMeshInstance *instances, uint32 instanceCount,             \
        RenderEffect *effect)
#define RENDERER_PUSH_TERRAIN(name)                                                                               \
    void name(RenderQueue *rq, Heightfield *heightfield, glm::vec2 heightmapSize, AssetHandle terrainShader,      \
        TextureHandle heightmapTexture, TextureHandle referenceHeightmapTexture,                                  \
        TextureHandle xAdjacentHeightmapTexture, TextureHandle xAdjacentReferenceHeightmapTexture,                \
        TextureHandle yAdjacentHeightmapTexture, TextureHandle yAdjacentReferenceHeightmapTexture,                \
        TextureHandle oppositeHeightmapTexture, TextureHandle oppositeReferenceHeightmapTexture,                  \
        uint32 materialCount, RenderTerrainMaterial *materials, bool isWireframe, uint32 visualizationMode,       \
        glm::vec2 cursorPos, float cursorRadius, float cursorFalloff)
#define RENDERER_DRAW_TO_TARGET(name) bool name(RenderQueue *rq, RenderTarget *target)
#define RENDERER_DRAW_TO_SCREEN(name) bool name(RenderQueue *rq, uint32 width, uint32 height)

#endif