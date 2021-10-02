#ifndef ENGINE_RENDERER_H
#define ENGINE_RENDERER_H

struct RenderContext;
struct RenderEffect;
struct RenderQueue;

struct RenderOutput
{
    uint32 width;
    uint32 height;
    RenderTarget *target;
};
#define getScreenRenderOutput(width, height)                                                                      \
    {                                                                                                             \
        width, height, 0                                                                                          \
    }

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
#define getBounds(renderTarget) rectMinDim(0, 0, renderTarget->width, renderTarget->height)
#define getRenderOutput(renderTarget)                                                                             \
    {                                                                                                             \
        renderTarget->width, renderTarget->height, renderTarget                                                   \
    }

// effects

#define RENDERER_CREATE_EFFECT(name)                                                                              \
    RenderEffect *name(MemoryArena *arena, AssetHandle shaderAsset, RenderEffectBlendMode blendMode)
#define RENDERER_CREATE_EFFECT_OVERRIDE(name) RenderEffect *name(RenderEffect *baseEffect)
#define RENDERER_SET_EFFECT_FLOAT(name) void name(RenderEffect *effect, char *paramName, float value)
#define RENDERER_SET_EFFECT_VEC2(name) void name(RenderEffect *effect, char *paramName, glm::vec2 value)
#define RENDERER_SET_EFFECT_VEC3(name) void name(RenderEffect *effect, char *paramName, glm::vec3 value)
#define RENDERER_SET_EFFECT_INT(name) void name(RenderEffect *effect, char *paramName, int32 value)
#define RENDERER_SET_EFFECT_UINT(name) void name(RenderEffect *effect, char *paramName, uint32 value)
#define RENDERER_SET_EFFECT_TEXTURE(name) void name(RenderEffect *effect, uint32 slot, TextureHandle handle)

// render queue

#define RENDERER_CREATE_QUEUE(name) RenderQueue *name(RenderContext *ctx, MemoryArena *arena, RenderOutput output)
#define RENDERER_SET_CAMERA_ORTHO(name) glm::mat4 name(RenderQueue *rq)
#define RENDERER_SET_CAMERA_ORTHO_OFFSET(name) glm::mat4 name(RenderQueue *rq, glm::vec2 cameraPos)
#define RENDERER_SET_CAMERA_PERSP(name)                                                                           \
    glm::mat4 name(RenderQueue *rq, glm::vec3 cameraPos, glm::vec3 lookAt, float fov)
#define RENDERER_SET_LIGHTING(name)                                                                               \
    void name(RenderQueue *rq, glm::vec4 *lightDir, bool isLightingEnabled, bool isTextureEnabled,                \
        bool isNormalMapEnabled, bool isAOMapEnabled, bool isDisplacementMapEnabled)
#define RENDERER_CLEAR(name) void name(RenderQueue *rq, float r, float g, float b, float a)
#define RENDERER_PUSH_TEXTURED_QUAD(name)                                                                         \
    void name(RenderQueue *rq, rect2 quad, TextureHandle textureHandle, bool isTopDown)
#define RENDERER_PUSH_TEXTURED_QUAD_REGION(name)                                                                  \
    void name(RenderQueue *rq, rect2 quad, TextureHandle textureHandle, bool isTopDown, rect2 uvRect)
#define RENDERER_PUSH_COLORED_QUAD(name) void name(RenderQueue *rq, rect2 quad, glm::vec3 color)
#define RENDERER_PUSH_QUAD(name) void name(RenderQueue *rq, rect2 quad, RenderEffect *effect)
#define RENDERER_PUSH_QUAD_BOTTOM_UP(name) void name(RenderQueue *rq, rect2 quad, RenderEffect *effect)
#define RENDERER_PUSH_QUADS(name) void name(RenderQueue *rq, rect2 *quads, uint32 quadCount, RenderEffect *effect)
#define RENDERER_PUSH_LINE(name) void name(RenderQueue *rq, glm::vec3 start, glm::vec3 end, glm::vec3 color)
#define RENDERER_BEGIN_LINE(name) void name(RenderQueue *rq, glm::vec3 start, glm::vec3 color)
#define RENDERER_EXTEND_LINE(name) void name(RenderQueue *rq, glm::vec3 point)
#define RENDERER_END_LINE(name) void name(RenderQueue *rq, glm::vec3 end)
#define RENDERER_END_LINE_LOOP(name) void name(RenderQueue *rq)
#define RENDERER_PUSH_QUAD_OUTLINE_XY(name) void name(RenderQueue *rq, rect2 quad, glm::vec3 color)
#define RENDERER_PUSH_QUAD_OUTLINE_XZ(name) void name(RenderQueue *rq, rect2 quad, glm::vec3 color)
#define RENDERER_PUSH_MESHES(name)                                                                                \
    void name(RenderQueue *rq, AssetHandle mesh, RenderMeshInstance *instances, uint32 instanceCount,             \
        RenderEffect *effect)
#define RENDERER_PUSH_TERRAIN(name)                                                                               \
    void name(RenderQueue *rq, Heightfield *heightfield, glm::vec2 heightmapSize, float heightmapOverlapInTexels, \
        AssetHandle terrainShader, TextureHandle heightmapTexture, TextureHandle referenceHeightmapTexture,       \
        uint32 materialCount, RenderTerrainMaterial *materials, bool isWireframe, uint32 visualizationMode,       \
        glm::vec2 cursorPos, float cursorRadius, float cursorFalloff)
#define RENDERER_DRAW(name) bool name(RenderQueue *rq)

#endif