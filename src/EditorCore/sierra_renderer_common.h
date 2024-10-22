#ifndef SIERRA_RENDERER_COMMON_H
#define SIERRA_RENDERER_COMMON_H

#include <glm/gtc/type_ptr.hpp>

// assets

typedef void *AssetHandle;
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
struct TextureSlotHandle
{
    void *ptr;
};
struct MeshHandle
{
    void *ptr;
};

// textures

struct GetPixelsRequest
{
    void *handle;
    uint32 pixelCount;
};
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

struct RenderMeshInstance
{
    uint32 id;
    glm::mat4 transform;
};
struct TextureAsset;
struct RenderTerrainMaterial
{
    glm::vec2 textureSizeInWorldUnits;

    AssetHandle albedoTexture;
    AssetHandle normalTexture;
    AssetHandle displacementTexture;
    AssetHandle aoTexture;

    float slopeStart;
    float slopeEnd;
    float altitudeStart;
    float altitudeEnd;
};

#endif