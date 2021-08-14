#ifndef ENGINE_RENDERER_COMMON_H
#define ENGINE_RENDERER_COMMON_H

#include <glm/gtc/type_ptr.hpp>

// assets

enum ShaderType
{
    SHADER_TYPE_QUAD,
    SHADER_TYPE_MESH,
    SHADER_TYPE_TERRAIN
};
struct ShaderHandle
{
    void *ptr;
};
enum TextureFormat
{
    TEXTURE_FORMAT_RGB8,
    TEXTURE_FORMAT_R8,
    TEXTURE_FORMAT_R16,
    TEXTURE_FORMAT_R8UI,
    TEXTURE_FORMAT_R16UI,
    TEXTURE_FORMAT_R32UI
};
struct TextureHandle
{
    void *ptr;
};
struct MeshHandle
{
    void *ptr;
};
struct TextureArrayHandle
{
    void *ptr;
};

// textures

struct GetPixelsResult
{
    void *pixels;
    uint32 count;
};

// render targets

struct RenderTarget
{
    uint32 width;
    uint32 height;

    TextureHandle textureHandle;
    TextureHandle depthTextureHandle;
};

// effects

enum RenderEffectBlendMode
{
    EFFECT_BLEND_ALPHA_BLEND,
    EFFECT_BLEND_ADDITIVE,
    EFFECT_BLEND_MAX
};

// render queue

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
struct TextureAsset;
struct RenderTerrainMaterial
{
    glm::vec2 textureSizeInWorldUnits;

    TextureAsset *albedoTextureAsset;
    TextureAsset *normalTextureAsset;
    TextureAsset *displacementTextureAsset;
    TextureAsset *aoTextureAsset;

    float slopeStart;
    float slopeEnd;
    float altitudeStart;
    float altitudeEnd;
};

#endif