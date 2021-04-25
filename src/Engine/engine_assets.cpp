#include "engine_assets.h"

#include <stb/stb_image.h>
#include <fast_obj/fast_obj.h>
#include "engine_renderer.h"

#define ASSET_GET_INDEX(id) (id & 0x0FFFFFFF)
#define ASSET_GET_TYPE(id) ((id & 0xF0000000) >> 28)

#define MAX_ASSETS 4096
#define MAX_DEPENDENCIES_PER_ASSET 32

RENDERER_CREATE_SHADER_PROGRAM(rendererCreateShaderProgram);
RENDERER_CREATE_SHADER(rendererCreateShader);

struct MeshAssetSlot
{
    bool isUpToDate;
    bool isLoadQueued;
    MeshAsset *asset;
};

struct TextureInfo
{
    const char *relativePath;
    bool is16Bit;
};
struct MeshInfo
{
    const char *relativePath;
};

struct AssetsState
{
    AssetRegistration registeredAssets[MAX_ASSETS];
    uint32 registeredAssetCount;

    MeshAssetSlot meshAssetSlots[ASSET_MESH_COUNT];
    MeshAsset meshAssets[ASSET_MESH_COUNT];

    uint64 dataStorageUsed;
};

void *pushAssetData(EngineMemory *memory, uint64 size)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    uint64 availableStorage =
        memory->assets.size - (sizeof(AssetsState) + state->dataStorageUsed);
    assert(availableStorage >= size);

    void *address =
        (uint8 *)memory->assets.baseAddress + sizeof(AssetsState) + state->dataStorageUsed;
    state->dataStorageUsed += size;

    return address;
}
#define pushAssetStruct(memory, type) (type *)pushAssetData(memory, sizeof(type))

MeshInfo getMeshInfo(uint32 assetId)
{
    MeshInfo info = {};
    switch (assetId)
    {
    case ASSET_MESH_ROCK:
        info.relativePath = "rock.obj";
        break;
    }
    return info;
}

AssetRegistration *registerAsset(EngineMemory *memory,
    AssetType type,
    const char *relativePath,
    uint32 *dependencyAssetIds,
    uint32 dependencyCount)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    AssetRegistration *reg = &state->registeredAssets[state->registeredAssetCount];
    reg->id = state->registeredAssetCount | (type << 28);

    if (relativePath)
    {
        reg->regType = ASSET_REG_FILE;
        reg->fileState = pushAssetStruct(memory, AssetFileState);

        const char *srcCursor = relativePath;
        while (*srcCursor)
        {
            srcCursor++;
        }
        uint32 length = srcCursor - relativePath;
        reg->fileState->relativePath = (char *)pushAssetData(memory, length + 1);

        srcCursor = relativePath;
        char *dstCursor = reg->fileState->relativePath;
        while (*srcCursor)
        {
            *dstCursor++ = *srcCursor++;
        }
        *dstCursor = 0;
    }
    else if (dependencyCount > 0)
    {
        assert(dependencyCount <= MAX_DEPENDENCIES_PER_ASSET);
        reg->regType = ASSET_REG_COMPOSITE;
        reg->compositeState = pushAssetStruct(memory, CompositeAssetState);
        reg->compositeState->dependencyCount = dependencyCount;
        reg->compositeState->dependencyAssetIds =
            (uint32 *)pushAssetData(memory, sizeof(uint32) * dependencyCount);
        for (uint32 i = 0; i < dependencyCount; i++)
        {
            reg->compositeState->dependencyAssetIds[i] = dependencyAssetIds[i];
        }

        reg->compositeState->dependencyVersions =
            (uint8 *)pushAssetData(memory, sizeof(uint8) * dependencyCount);
    }
    else
    {
        reg->regType = ASSET_REG_VIRTUAL;
    }

    state->registeredAssetCount++;
    return reg;
}

ASSETS_REGISTER_TEXTURE(assetsRegisterTexture)
{
    AssetRegistration *reg = registerAsset(memory, ASSET_TYPE_TEXTURE, relativePath, 0, 0);
    reg->metadata.texture = pushAssetStruct(memory, TextureAssetMetadata);
    reg->metadata.texture->is16Bit = is16Bit;
    return reg->id;
}

ASSETS_REGISTER_SHADER(assetsRegisterShader)
{
    AssetRegistration *reg = registerAsset(memory, ASSET_TYPE_SHADER, relativePath, 0, 0);
    reg->metadata.shader = pushAssetStruct(memory, ShaderAssetMetadata);
    reg->metadata.shader->type = type;
    return reg->id;
}

ASSETS_REGISTER_SHADER_PROGRAM(assetsRegisterShaderProgram)
{
    AssetRegistration *reg =
        registerAsset(memory, ASSET_TYPE_SHADER_PROGRAM, 0, shaderAssetIds, shaderCount);
    return reg->id;
}

ASSETS_GET_REGISTERED_ASSET_COUNT(assetsGetRegisteredAssetCount)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;
    return state->registeredAssetCount;
}

ASSETS_GET_REGISTERED_ASSETS(assetsGetRegisteredAssets)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;
    return state->registeredAssets;
}

bool buildCompositeAsset(EngineMemory *memory, AssetRegistration *reg, LoadedAsset **deps)
{
    uint32 assetType = ASSET_GET_TYPE(reg->id);
    if (assetType == ASSET_TYPE_SHADER_PROGRAM)
    {
        uint32 shaderHandles[MAX_DEPENDENCIES_PER_ASSET];
        for (uint32 i = 0; i < reg->compositeState->dependencyCount; i++)
        {
            shaderHandles[i] = deps[i]->shader->handle;
        }

        uint32 handle;
        if (rendererCreateShaderProgram(
                memory, reg->compositeState->dependencyCount, shaderHandles, &handle))
        {
            if (!reg->asset.shaderProgram)
            {
                reg->asset.shaderProgram = pushAssetStruct(memory, ShaderProgramAsset);
            }
            reg->asset.shaderProgram->handle = handle;

            return true;
        }
    }
    return false;
}

LoadedAsset *getAsset(EngineMemory *memory, uint32 assetId)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    assert(assetIdx < state->registeredAssetCount);

    AssetRegistration *reg = &state->registeredAssets[assetIdx];
    if (reg->regType == ASSET_REG_FILE)
    {
        AssetFileState *fileState = reg->fileState;
        if (fileState->relativePath && !fileState->isUpToDate && !fileState->isLoadQueued)
        {
            if (memory->platformLoadAsset(assetId, fileState->relativePath))
            {
                fileState->isLoadQueued = true;
            }
        }
    }
    else if (reg->regType == ASSET_REG_COMPOSITE)
    {
        CompositeAssetState *compState = reg->compositeState;
        LoadedAsset *dependencies[MAX_DEPENDENCIES_PER_ASSET];

        bool shouldRebuildAsset = false;
        for (uint32 i = 0; i < compState->dependencyCount; i++)
        {
            LoadedAsset *dependency = getAsset(memory, compState->dependencyAssetIds[i]);
            if (!dependency->untyped)
            {
                shouldRebuildAsset = false;
                break;
            }
            if (dependency->version != compState->dependencyVersions[i])
            {
                shouldRebuildAsset = true;
            }
            dependencies[i] = dependency;
        }

        if (shouldRebuildAsset)
        {
            if (buildCompositeAsset(memory, reg, dependencies))
            {
                reg->asset.version++;
            }
            for (uint32 i = 0; i < compState->dependencyCount; i++)
            {
                compState->dependencyVersions[i] = dependencies[i]->version;
            }
        }
    }

    return &reg->asset;
}

ASSETS_GET_SHADER(assetsGetShader)
{
    assert(ASSET_GET_TYPE(assetId) == ASSET_TYPE_SHADER);
    return getAsset(memory, assetId);
}

ASSETS_GET_SHADER_PROGRAM(assetsGetShaderProgram)
{
    assert(ASSET_GET_TYPE(assetId) == ASSET_TYPE_SHADER_PROGRAM);
    return getAsset(memory, assetId);
}

ASSETS_LOAD_TEXTURE(assetsLoadTexture)
{
    const stbi_uc *rawData = static_cast<stbi_uc *>(data);
    void *loadedData;
    int32 width;
    int32 height;
    int32 channels;
    uint64 elementSize = 0;
    if (is16Bit)
    {
        loadedData = stbi_load_16_from_memory(rawData, size, &width, &height, &channels, 0);
        elementSize = 2;
    }
    else
    {
        loadedData = stbi_load_from_memory(rawData, size, &width, &height, &channels, 0);
        elementSize = 1;
    }
    assert(width >= 0);
    assert(height >= 0);

    out_asset->width = (uint32)width;
    out_asset->height = (uint32)height;

    uint64 requiredStorage = (uint64)width * (uint64)height * (uint64)channels * elementSize;
    out_asset->data = (uint8 *)pushAssetData(memory, requiredStorage);
    memcpy(out_asset->data, loadedData, requiredStorage);

    stbi_image_free(loadedData);
}

ASSETS_GET_TEXTURE(assetsGetTexture)
{
    assert(ASSET_GET_TYPE(assetId) == ASSET_TYPE_TEXTURE);
    return getAsset(memory, assetId);
}

struct FastObjVirtualFile
{
    void *data;
    uint64 size;
    uint64 position;
};
void *fastObjFileOpen(const char *path, void *user_data)
{
    return user_data;
}
void fastObjFileClose(void *file, void *user_data)
{
    FastObjVirtualFile *virtualFile = (FastObjVirtualFile *)file;
    virtualFile->position = 0;
}
uint64 fastObjFileRead(void *file, void *dst, uint64 bytes, void *user_data)
{
    FastObjVirtualFile *virtualFile = (FastObjVirtualFile *)file;
    uint64 size = bytes;
    uint64 remainingBytes = virtualFile->size - virtualFile->position;
    if (size > remainingBytes)
    {
        size = remainingBytes;
    }
    memcpy(dst, (uint8 *)virtualFile->data + virtualFile->position, size);
    virtualFile->position += size;
    return size;
}
unsigned long fastObjFileSize(void *file, void *user_data)
{
    FastObjVirtualFile *virtualFile = (FastObjVirtualFile *)file;
    return virtualFile->size;
}
void fastObjLoadMesh(
    EngineMemory *memory, const char *path, void *data, uint64 size, MeshAsset *out_asset)
{
    fastObjCallbacks callbacks = {};
    callbacks.file_open = fastObjFileOpen;
    callbacks.file_close = fastObjFileClose;
    callbacks.file_read = fastObjFileRead;
    callbacks.file_size = fastObjFileSize;

    FastObjVirtualFile virtualFile = {};
    virtualFile.data = data;
    virtualFile.size = size;

    fastObjMesh *mesh = fast_obj_read_with_callbacks(path, &callbacks, &virtualFile);

    out_asset->elementCount = mesh->face_count * 3;
    uint32 elementBufferSize = sizeof(uint32) * out_asset->elementCount;
    out_asset->indices = (uint32 *)pushAssetData(memory, elementBufferSize);

    out_asset->vertexCount = out_asset->elementCount;
    uint32 vertexBufferSize = sizeof(float) * out_asset->vertexCount * 6;
    out_asset->vertices = (float *)pushAssetData(memory, vertexBufferSize);

    uint32 *currentIndex = (uint32 *)out_asset->indices;
    float *currentVertex = (float *)out_asset->vertices;
    for (uint32 i = 0; i < out_asset->elementCount; i++)
    {
        fastObjIndex index = mesh->indices[i];
        *currentIndex++ = i;
        *currentVertex++ = mesh->positions[index.p * 3];
        *currentVertex++ = mesh->positions[(index.p * 3) + 1];
        *currentVertex++ = mesh->positions[(index.p * 3) + 2];
        *currentVertex++ = mesh->normals[index.n * 3];
        *currentVertex++ = mesh->normals[(index.n * 3) + 1];
        *currentVertex++ = mesh->normals[(index.n * 3) + 2];
    }

    fast_obj_destroy(mesh);
}

ASSETS_GET_MESH(assetsGetMesh)
{
    assert(ASSET_GET_TYPE(assetId) == ASSET_TYPE_MESH);
    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    assert(assetIdx < ASSET_MESH_COUNT);

    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    MeshAssetSlot *slot = &state->meshAssetSlots[assetIdx];
    if (!slot->isUpToDate && !slot->isLoadQueued)
    {
        MeshInfo meshInfo = getMeshInfo(assetId);
        if (memory->platformLoadAsset(assetId, meshInfo.relativePath))
        {
            slot->isLoadQueued = true;
        }
    }

    return slot->asset;
}

ASSETS_ON_ASSET_LOADED(assetsOnAssetLoaded)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    uint32 assetType = ASSET_GET_TYPE(assetId);

    if (assetType == ASSET_TYPE_SHADER || assetType == ASSET_TYPE_TEXTURE)
    {
        assert(assetIdx < state->registeredAssetCount);
        AssetRegistration *reg = &state->registeredAssets[assetIdx];

        if (assetType == ASSET_TYPE_SHADER)
        {
            if (!reg->asset.shader)
            {
                reg->asset.shader = pushAssetStruct(memory, ShaderAsset);
            }

            char *src = static_cast<char *>(data);
            uint32 handle;
            if (rendererCreateShader(memory, reg->metadata.shader->type, src, &handle))
            {
                reg->asset.shader->handle = handle;
            }
        }
        else if (assetType == ASSET_TYPE_TEXTURE)
        {
            if (!reg->asset.texture)
            {
                reg->asset.texture = pushAssetStruct(memory, TextureAsset);
            }
            assetsLoadTexture(
                memory, data, size, reg->metadata.texture->is16Bit, reg->asset.texture);
        }
        reg->asset.version++;
        if (reg->regType == ASSET_REG_FILE)
        {
            reg->fileState->isUpToDate = true;
        }
    }
    else if (assetType == ASSET_TYPE_MESH)
    {
        assert(assetIdx < ASSET_MESH_COUNT);
        MeshInfo meshInfo = getMeshInfo(assetId);

        MeshAsset *asset = &state->meshAssets[assetIdx];
        fastObjLoadMesh(memory, meshInfo.relativePath, data, size, asset);
        asset->version++;

        MeshAssetSlot *slot = &state->meshAssetSlots[assetIdx];
        slot->asset = asset;
        slot->isUpToDate = true;
    }
}

ASSETS_INVALIDATE_ASSET(assetsInvalidateAsset)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    uint32 assetType = ASSET_GET_TYPE(assetId);

    if (assetType == ASSET_TYPE_SHADER || assetType == ASSET_TYPE_TEXTURE)
    {
        assert(assetIdx < state->registeredAssetCount);
        AssetRegistration *reg = &state->registeredAssets[assetIdx];
        if (reg->regType == ASSET_REG_FILE)
        {
            reg->fileState->isUpToDate = false;
            reg->fileState->isLoadQueued = false;
        }
    }
    else if (assetType == ASSET_TYPE_MESH)
    {
        assert(assetIdx < ASSET_MESH_COUNT);
        MeshAssetSlot *slot = &state->meshAssetSlots[assetIdx];
        slot->isUpToDate = false;
        slot->isLoadQueued = false;
    }
}