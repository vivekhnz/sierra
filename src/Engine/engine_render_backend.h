#ifndef ENGINE_RENDER_BACKEND_H
#define ENGINE_RENDER_BACKEND_H

#include "engine_platform.h"
#include "engine_renderer_common.h"
#include "engine_heightfield.h"

struct RenderBackendContext
{
    void *ptr;
};

enum RenderEffectParameterType
{
    EFFECT_PARAM_TYPE_FLOAT,
    EFFECT_PARAM_TYPE_VEC3,
    EFFECT_PARAM_TYPE_INT,
    EFFECT_PARAM_TYPE_UINT
};
struct RenderEffectParameter
{
    char *name;
    RenderEffectParameterType type;
    union
    {
        float f;
        int32 i;
        uint32 u;
        glm::vec3 v3;
    } value;
    RenderEffectParameter *next;
};
struct RenderEffectTexture
{
    uint32 slot;
    TextureHandle handle;
    RenderEffectTexture *next;
};
struct RenderEffect
{
    MemoryArena *arena;
    ShaderHandle shaderHandle;
    RenderEffectBlendMode blendMode;
    RenderEffectParameter *firstParameter;
    RenderEffectParameter *lastParameter;
    RenderEffectTexture *firstTexture;
    RenderEffectTexture *lastTexture;
};

enum RenderQueueCommandType
{
    RENDER_CMD_SetCameraCommand,
    RENDER_CMD_SetLightingCommand,
    RENDER_CMD_ClearCommand,
    RENDER_CMD_DrawQuadsCommand,
    RENDER_CMD_DrawMeshesCommand,
    RENDER_CMD_DrawTerrainCommand
};
struct RenderQueueCommandHeader
{
    RenderQueueCommandType type;
    RenderQueueCommandHeader *next;
};

struct SetLightingCommand
{
    glm::vec4 lightDir;

    uint32 isEnabled;
    uint32 isTextureEnabled;
    uint32 isNormalMapEnabled;
    uint32 isAOMapEnabled;
    uint32 isDisplacementMapEnabled;
};
struct ClearCommand
{
    glm::vec4 color;
};
struct DrawQuadsCommand
{
    RenderEffect *effect;
    bool isTopDown;
    uint32 instanceOffset;
    uint32 instanceCount;
};
struct DrawMeshesCommand
{
    RenderEffect *effect;
    MeshHandle mesh;
    uint32 instanceOffset;
    uint32 instanceCount;
};
struct DrawTerrainCommand
{
    Heightfield *heightfield;
    glm::vec2 heightmapSize;

    ShaderHandle terrainShader;

    TextureHandle heightmapTexture;
    TextureHandle referenceHeightmapTexture;
    TextureHandle xAdjacentHeightmapTexture;
    TextureHandle xAdjacentReferenceHeightmapTexture;
    TextureHandle yAdjacentHeightmapTexture;
    TextureHandle yAdjacentReferenceHeightmapTexture;
    TextureHandle oppositeHeightmapTexture;
    TextureHandle oppositeReferenceHeightmapTexture;

    uint32 meshVertexBufferId;
    uint32 meshElementBufferId;
    uint32 tessellationLevelBufferId;
    uint32 meshElementCount;

    uint32 materialCount;
    uint32 albedoTextureArrayId;
    uint32 normalTextureArrayId;
    uint32 displacementTextureArrayId;
    uint32 aoTextureArrayId;
    uint32 materialPropsBufferId;

    bool isWireframe;

    uint32 visualizationMode;
    glm::vec2 cursorPos;
    float cursorRadius;
    float cursorFalloff;
};

struct DispatchedRenderQueue
{
    RenderBackendContext ctx;

    RenderQuad *quads;
    uint32 quadCount;

    RenderMeshInstance *meshInstances;
    uint32 meshInstanceCount;

    RenderQueueCommandHeader *firstCommand;
};

RenderBackendContext initializeRenderBackend(MemoryArena *arena);

bool createShader(RenderBackendContext rctx, ShaderType type, char *src, ShaderHandle *out_handle);
void destroyShader(ShaderHandle handle);

ShaderHandle getTexturedQuadShader(RenderBackendContext rctx);
ShaderHandle getColoredQuadShader(RenderBackendContext rctx);

MeshHandle createMesh(MemoryArena *arena, void *vertices, uint32 vertexCount, void *indices, uint32 indexCount);
void destroyMesh(MeshHandle handle);

TextureHandle createTexture(uint32 width, uint32 height, TextureFormat format);
void updateTexture(TextureHandle handle, uint32 width, uint32 height, void *pixels);
void *getPixels(MemoryArena *arena, TextureHandle handle, uint32 width, uint32 height, uint32 *out_pixelCount);
void *getPixelsInRegion(MemoryArena *arena,
    TextureHandle handle,
    uint32 x,
    uint32 y,
    uint32 width,
    uint32 height,
    uint32 *out_pixelCount);

RenderTarget *createRenderTarget(
    MemoryArena *arena, uint32 width, uint32 height, TextureFormat format, bool createDepthBuffer);
void resizeRenderTarget(RenderTarget *target, uint32 width, uint32 height);

bool drawToTarget(DispatchedRenderQueue *rq, uint32 width, uint32 height, RenderTarget *target);

#endif