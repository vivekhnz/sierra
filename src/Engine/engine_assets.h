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

struct ShaderAsset
{
    uint32 handle;
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
        MeshAsset *mesh;
    };
};

struct ShaderAssetMetadata
{
    uint32 type;
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
        ShaderAssetMetadata *shader;
        TextureAssetMetadata *texture;
    } metadata;
    LoadedAsset asset;
};

#define ASSETS_REGISTER_TEXTURE(name)                                                         \
    uint32 name(EngineMemory *memory, const char *relativePath, bool is16Bit)
typedef ASSETS_REGISTER_TEXTURE(AssetsRegisterTexture);

#define ASSETS_REGISTER_SHADER(name)                                                          \
    uint32 name(EngineMemory *memory, const char *relativePath, uint32 type)
typedef ASSETS_REGISTER_SHADER(AssetsRegisterShader);

#define ASSETS_REGISTER_SHADER_PROGRAM(name)                                                  \
    uint32 name(EngineMemory *memory, uint32 *shaderAssetIds, uint32 shaderCount)
typedef ASSETS_REGISTER_SHADER_PROGRAM(AssetsRegisterShaderProgram);

#define ASSETS_REGISTER_MESH(name) uint32 name(EngineMemory *memory, const char *relativePath)
typedef ASSETS_REGISTER_MESH(AssetsRegisterMesh);

#define ASSETS_GET_REGISTERED_ASSET_COUNT(name) uint32 name(EngineMemory *memory)
typedef ASSETS_GET_REGISTERED_ASSET_COUNT(AssetsGetRegisteredAssetCount);

#define ASSETS_GET_REGISTERED_ASSETS(name) AssetRegistration *name(EngineMemory *memory)
typedef ASSETS_GET_REGISTERED_ASSETS(AssetsGetRegisteredAssets);

#define ASSETS_GET_SHADER(name) LoadedAsset *name(EngineMemory *memory, uint32 assetId)
typedef ASSETS_GET_SHADER(AssetsGetShader);

#define ASSETS_GET_SHADER_PROGRAM(name) LoadedAsset *name(EngineMemory *memory, uint32 assetId)
typedef ASSETS_GET_SHADER_PROGRAM(AssetsGetShaderProgram);

#define ASSETS_LOAD_TEXTURE(name)                                                             \
    void name(                                                                                \
        EngineMemory *memory, void *data, uint64 size, bool is16Bit, TextureAsset *out_asset)
typedef ASSETS_LOAD_TEXTURE(AssetsLoadTexture);

#define ASSETS_GET_TEXTURE(name) LoadedAsset *name(EngineMemory *memory, uint32 assetId)
typedef ASSETS_GET_TEXTURE(AssetsGetTexture);

#define ASSETS_GET_MESH(name) LoadedAsset *name(EngineMemory *memory, uint32 assetId)
typedef ASSETS_GET_MESH(AssetsGetMesh);

#define ASSETS_SET_ASSET_DATA(name)                                                           \
    void name(EngineMemory *memory, uint32 assetId, void *data, uint64 size)
typedef ASSETS_SET_ASSET_DATA(AssetsSetAssetData);

#define ASSETS_INVALIDATE_ASSET(name) void name(EngineMemory *memory, uint32 assetId)
typedef ASSETS_INVALIDATE_ASSET(AssetsInvalidateAsset);

#endif