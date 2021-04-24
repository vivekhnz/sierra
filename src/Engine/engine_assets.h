#ifndef ENGINE_ASSETS_H
#define ENGINE_ASSETS_H

#include "engine_platform.h"

enum AssetType
{
    ASSET_TYPE_SHADER = 1,
    ASSET_TYPE_SHADER_PROGRAM,
    ASSET_TYPE_TEXTURE,
    ASSET_TYPE_MESH
};

#define ASSET_ID(type, name, idx) ASSET_##type##_##name## = idx | (ASSET_TYPE_##type## << 28)

enum ShaderAssetId
{
    ASSET_ID(SHADER, TEXTURE_VERTEX, 0),
    ASSET_ID(SHADER, TEXTURE_FRAGMENT, 1),
    ASSET_ID(SHADER, TERRAIN_VERTEX, 2),
    ASSET_ID(SHADER, TERRAIN_TESS_CTRL, 3),
    ASSET_ID(SHADER, TERRAIN_TESS_EVAL, 4),
    ASSET_ID(SHADER, TERRAIN_FRAGMENT, 5),
    ASSET_ID(SHADER, TERRAIN_COMPUTE_TESS_LEVEL, 6),
    ASSET_ID(SHADER, WIREFRAME_VERTEX, 7),
    ASSET_ID(SHADER, WIREFRAME_TESS_CTRL, 8),
    ASSET_ID(SHADER, WIREFRAME_TESS_EVAL, 9),
    ASSET_ID(SHADER, WIREFRAME_FRAGMENT, 10),
    ASSET_ID(SHADER, BRUSH_MASK_VERTEX, 11),
    ASSET_ID(SHADER, BRUSH_MASK_FRAGMENT, 12),
    ASSET_ID(SHADER, BRUSH_BLEND_ADD_SUB_FRAGMENT, 13),
    ASSET_ID(SHADER, BRUSH_BLEND_FLATTEN_FRAGMENT, 14),
    ASSET_ID(SHADER, BRUSH_BLEND_SMOOTH_FRAGMENT, 15),
    ASSET_ID(SHADER, ROCK_VERTEX, 16),
    ASSET_ID(SHADER, ROCK_FRAGMENT, 17)
};
#define ASSET_SHADER_COUNT 18

enum MeshAssetId
{
    ASSET_ID(MESH, ROCK, 0)
};
#define ASSET_MESH_COUNT 12

struct ShaderAsset
{
    uint32 handle;
    uint8 version;
};
struct ShaderProgramAsset
{
    uint32 handle;
};
struct TextureAsset
{
    uint32 width;
    uint32 height;
    void *data;
};
struct MeshAsset
{
    uint8 version;
    uint32 vertexCount;
    uint32 elementCount;
    void *vertices;
    void *indices;
};
struct LoadedAsset
{
    uint8 version;
    union
    {
        void *untyped;
        ShaderAsset *shader;
        ShaderProgramAsset *shaderProgram;
        TextureAsset *texture;
    };
};

struct TextureAssetMetadata
{
    bool is16Bit;
};

struct AssetFileState
{
    char *relativePath;
    bool isUpToDate;
    bool isLoadQueued;
};
struct CompositeAssetState
{
    uint32 dependencyCount;
    uint32 *dependencyAssetIds;
    uint8 *dependencyVersions;
};

enum AssetRegistrationType
{
    ASSET_REG_FILE,
    ASSET_REG_COMPOSITE,
    ASSET_REG_VIRTUAL
};
struct AssetRegistration
{
    uint32 id;
    AssetRegistrationType regType;
    union
    {
        AssetFileState *fileState;
        CompositeAssetState *compositeState;
    };
    union
    {
        TextureAssetMetadata *texture;
    } metadata;
    LoadedAsset asset;
};

#define ASSETS_REGISTER_TEXTURE(name)                                                         \
    uint32 name(EngineMemory *memory, const char *relativePath, bool is16Bit)
typedef ASSETS_REGISTER_TEXTURE(AssetsRegisterTexture);

#define ASSETS_REGISTER_SHADER_PROGRAM(name)                                                  \
    uint32 name(EngineMemory *memory, uint32 *shaderAssetIds, uint32 shaderCount)
typedef ASSETS_REGISTER_SHADER_PROGRAM(AssetsRegisterShaderProgram);

#define ASSETS_GET_REGISTERED_ASSET_COUNT(name) uint32 name(EngineMemory *memory)
typedef ASSETS_GET_REGISTERED_ASSET_COUNT(AssetsGetRegisteredAssetCount);

#define ASSETS_GET_REGISTERED_ASSETS(name) AssetRegistration *name(EngineMemory *memory)
typedef ASSETS_GET_REGISTERED_ASSETS(AssetsGetRegisteredAssets);

#define ASSETS_GET_SHADER(name) ShaderAsset *name(EngineMemory *memory, uint32 assetId)
typedef ASSETS_GET_SHADER(AssetsGetShader);

#define ASSETS_GET_SHADER_PROGRAM(name) LoadedAsset *name(EngineMemory *memory, uint32 assetId)
typedef ASSETS_GET_SHADER_PROGRAM(AssetsGetShaderProgram);

#define ASSETS_LOAD_TEXTURE(name)                                                             \
    void name(                                                                                \
        EngineMemory *memory, void *data, uint64 size, bool is16Bit, TextureAsset *out_asset)
typedef ASSETS_LOAD_TEXTURE(AssetsLoadTexture);

#define ASSETS_GET_TEXTURE(name) LoadedAsset *name(EngineMemory *memory, uint32 assetId)
typedef ASSETS_GET_TEXTURE(AssetsGetTexture);

#define ASSETS_GET_MESH(name) MeshAsset *name(EngineMemory *memory, uint32 assetId)
typedef ASSETS_GET_MESH(AssetsGetMesh);

#define ASSETS_ON_ASSET_LOADED(name)                                                          \
    void name(EngineMemory *memory, uint32 assetId, void *data, uint64 size)
typedef ASSETS_ON_ASSET_LOADED(AssetsOnAssetLoaded);

#define ASSETS_INVALIDATE_ASSET(name) void name(EngineMemory *memory, uint32 assetId)
typedef ASSETS_INVALIDATE_ASSET(AssetsInvalidateAsset);

#endif