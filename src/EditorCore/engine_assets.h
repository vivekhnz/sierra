#ifndef ENGINE_ASSETS_H
#define ENGINE_ASSETS_H

enum AssetType
{
    ASSET_TYPE_SHADER = 1,
    ASSET_TYPE_TEXTURE,
    ASSET_TYPE_MESH
};

struct ShaderAsset
{
    ShaderHandle handle;
};
struct TextureAsset
{
    uint32 width;
    uint32 height;
    void *data;
    TextureSlotHandle slot;
};
struct MeshAsset
{
    uint32 vertexCount;
    uint32 elementCount;
    void *vertices;
    void *indices;
    MeshHandle handle;
};
struct LoadedAsset
{
    uint8 version;
    union
    {
        void *untyped;
        ShaderAsset *shader;
        TextureAsset *texture;
        MeshAsset *mesh;
    };
};

enum ShaderType;
struct ShaderAssetMetadata
{
    ShaderType type;
};
struct TextureAssetMetadata
{
    TextureFormat format;
};

struct AssetFileState
{
    char *relativePath;
    bool isUpToDate;
    bool isLoadQueued;
    uint64 lastWriteTime;
};

enum AssetRegistrationType
{
    ASSET_REG_FILE,
    ASSET_REG_VIRTUAL
};
struct AssetHandleInternal;
struct AssetRegistration
{
    AssetHandleInternal *handle;
    AssetRegistrationType regType;
    AssetFileState *fileState;
    AssetType assetType;
    union
    {
        ShaderAssetMetadata *shader;
        TextureAssetMetadata *texture;
    } metadata;
    LoadedAsset asset;
};

struct Assets;

#endif