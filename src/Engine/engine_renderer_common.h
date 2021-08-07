#ifndef ENGINE_RENDERER_COMMON_H
#define ENGINE_RENDERER_COMMON_H

#include <glm/gtc/type_ptr.hpp>

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

struct RenderTarget
{
    uint32 width;
    uint32 height;

    TextureHandle textureHandle;
    TextureHandle depthTextureHandle;
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

enum RenderEffectBlendMode
{
    EFFECT_BLEND_ALPHA_BLEND,
    EFFECT_BLEND_ADDITIVE,
    EFFECT_BLEND_MAX
};

// todo: move to engine_render_backend.h
struct SetCameraCommand
{
    bool isOrthographic;

    glm::vec3 cameraPos;
    glm::vec3 lookAt;
    float fov;
};

#endif