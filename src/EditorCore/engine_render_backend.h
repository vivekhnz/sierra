#ifndef ENGINE_RENDER_BACKEND_H
#define ENGINE_RENDER_BACKEND_H

#include "engine_platform.h"
#include "engine_renderer_common.h"

struct RenderBackendContext
{
    void *ptr;
};

enum RenderEffectParameterType
{
    EFFECT_PARAM_TYPE_FLOAT,
    EFFECT_PARAM_TYPE_VEC2,
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
        glm::vec2 v2;
        glm::vec3 v3;
    } value;
};
struct RenderEffectParameterLink
{
    RenderEffectParameter *param;
    RenderEffectParameterLink *next;
};
struct RenderEffectTexture
{
    uint32 slot;
    TextureHandle handle;
};
struct RenderEffectTextureLink
{
    RenderEffectTexture *texture;
    RenderEffectTextureLink *next;
};
struct RenderEffect
{
    MemoryArena *arena;
    ShaderHandle shaderHandle;
    RenderEffectBlendMode blendMode;
    RenderEffectParameterLink *firstParameter;
    RenderEffectParameterLink *lastParameter;
    RenderEffectTextureLink *firstTexture;
    RenderEffectTextureLink *lastTexture;
};

enum RenderQueueCommandType
{
    RENDER_CMD_SetCameraCommand,
    RENDER_CMD_SetLightingCommand,
    RENDER_CMD_ClearCommand,
    RENDER_CMD_DrawQuadsCommand,
    RENDER_CMD_DrawLineCommand,
    RENDER_CMD_DrawMeshesCommand,
    RENDER_CMD_DrawTerrainCommand
};
struct RenderQueueCommandHeader
{
    RenderQueueCommandType type;
    RenderQueueCommandHeader *next;
};

struct SetCameraCommand
{
    glm::mat4 transform;
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
struct DrawLineCommand
{
    uint32 vertexIndex;
    uint32 vertexCount;
    glm::vec3 color;
};
struct DrawMeshesCommand
{
    RenderEffect *effect;
    MeshHandle mesh;
    uint32 instanceOffset;
    uint32 instanceCount;
};
struct ResolvedTerrainMaterial
{
    glm::vec2 textureSizeInWorldUnits;

    TextureAsset *albedoTexture;
    TextureAsset *normalTexture;
    TextureAsset *displacementTexture;
    TextureAsset *aoTexture;

    float slopeStart;
    float slopeEnd;
    float altitudeStart;
    float altitudeEnd;
};
struct DrawTerrainCommand
{
    glm::vec2 heightfieldCenter;
    float heightfieldMaxHeight;

    glm::vec2 heightmapSize;
    float heightmapOverlapInTexels;

    ShaderHandle terrainShader;

    TextureHandle heightmapTexture;
    TextureHandle referenceHeightmapTexture;

    uint32 materialCount;
    ResolvedTerrainMaterial *materials;

    bool isWireframe;

    uint32 visualizationMode;
    glm::vec2 cursorPos;
    float cursorRadius;
    float cursorFalloff;
};

struct DispatchedRenderQueue
{
    RenderBackendContext ctx;

    rect2 *quads;
    uint32 quadCount;

    glm::vec3 *primitiveVertices;
    uint32 primitiveVertexCount;

    RenderMeshInstance *meshInstances;
    uint32 meshInstanceCount;

    RenderQueueCommandHeader *firstCommand;
};

void renderBackendReload();
RenderBackendContext renderBackendInitialize(MemoryArena *arena);

bool renderBackendCreateShader(RenderBackendContext rctx, ShaderType type, char *src, ShaderHandle *out_handle);
void renderBackendDestroyShader(ShaderHandle handle);

ShaderHandle renderBackendGetTexturedQuadShader(RenderBackendContext rctx);
ShaderHandle renderBackendGetColoredQuadShader(RenderBackendContext rctx);

MeshHandle renderBackendCreateMesh(
    MemoryArena *arena, void *vertices, uint32 vertexCount, void *indices, uint32 indexCount);
void renderBackendDestroyMesh(MeshHandle handle);

uint32 renderBackendGetTextureElementSize(TextureFormat format);
TextureHandle renderBackendCreateTexture(uint32 width, uint32 height, TextureFormat format);
void renderBackendUpdateTexture(TextureHandle handle, uint32 width, uint32 height, void *pixels);
GetPixelsResult renderBackendGetPixels(MemoryArena *arena, TextureHandle handle, uint32 width, uint32 height);
GetPixelsResult renderBackendGetPixelsInRegion(
    MemoryArena *arena, TextureHandle handle, uint32 x, uint32 y, uint32 width, uint32 height);

RenderTarget *renderBackendCreateRenderTarget(
    MemoryArena *arena, uint32 width, uint32 height, TextureFormat format, bool createDepthBuffer);
void renderBackendResizeRenderTarget(RenderTarget *target, uint32 width, uint32 height);

TextureSlotHandle renderBackendReserveTextureSlot(
    RenderBackendContext rctx, uint32 width, uint32 height, TextureFormat format);
void renderBackendUpdateTextureSlot(TextureSlotHandle handle, void *pixels);

bool renderBackendDrawToOutput(DispatchedRenderQueue *rq, RenderOutput *output);

#endif