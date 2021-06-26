#include "engine_assets.h"

#include <stb/stb_image.h>
#include <fast_obj/fast_obj.h>
#include "engine_renderer.h"

extern EnginePlatformApi Platform;

#define ASSET_GET_INDEX(id) (id & 0x0FFFFFFF)
#define ASSET_GET_TYPE(id) ((id & 0xF0000000) >> 28)

#define MAX_ASSETS 4096
#define MAX_DEPENDENCIES_PER_ASSET 32

RENDERER_CREATE_SHADER_PROGRAM(rendererCreateShaderProgram);
RENDERER_CREATE_SHADER(rendererCreateShader);

struct AssetsState
{
    MemoryBlock *data;
    RenderContext *renderCtx;

    AssetRegistration registeredAssets[MAX_ASSETS];
    uint32 registeredAssetCount;

    uint64 dataStorageUsed;
};
struct AssetHandleInternal
{
    uint32 id;
    AssetsState *state;
};

void *pushAssetData(MemoryBlock *memory, uint64 size)
{
    assert(memory->size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->baseAddress;

    uint64 availableStorage = memory->size - (sizeof(AssetsState) + state->dataStorageUsed);
    assert(availableStorage >= size);

    void *address =
        (uint8 *)memory->baseAddress + sizeof(AssetsState) + state->dataStorageUsed;
    state->dataStorageUsed += size;

    return address;
}
#define pushAssetStruct(memory, type) (type *)pushAssetData(memory, sizeof(type))

AssetRegistration *registerAsset(EngineMemory *memory,
    AssetType type,
    const char *relativePath,
    AssetHandle *dependencyAssetHandles,
    uint32 dependencyCount)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    AssetRegistration *reg = &state->registeredAssets[state->registeredAssetCount];
    reg->handle = pushAssetStruct(&memory->assets, AssetHandleInternal);
    reg->handle->id = state->registeredAssetCount | (type << 28);
    reg->handle->state = state;
    reg->assetType = type;

    if (relativePath)
    {
        reg->regType = ASSET_REG_FILE;
        reg->fileState = pushAssetStruct(&memory->assets, AssetFileState);

        const char *srcCursor = relativePath;
        while (*srcCursor)
        {
            srcCursor++;
        }
        uint32 length = srcCursor - relativePath;
        reg->fileState->relativePath = (char *)pushAssetData(&memory->assets, length + 1);

        srcCursor = relativePath;
        char *dstCursor = reg->fileState->relativePath;
        while (*srcCursor)
        {
            *dstCursor++ = *srcCursor++;
        }
        *dstCursor = 0;

        Platform.watchAssetFile(reg->handle, relativePath);
    }
    else if (dependencyCount > 0)
    {
        assert(dependencyCount <= MAX_DEPENDENCIES_PER_ASSET);
        reg->regType = ASSET_REG_COMPOSITE;
        reg->compositeState = pushAssetStruct(&memory->assets, CompositeAssetState);
        reg->compositeState->dependencyCount = dependencyCount;
        reg->compositeState->dependencyAssetIds =
            (uint32 *)pushAssetData(&memory->assets, sizeof(uint32) * dependencyCount);
        for (uint32 i = 0; i < dependencyCount; i++)
        {
            AssetHandleInternal *handle = (AssetHandleInternal *)dependencyAssetHandles[i];
            reg->compositeState->dependencyAssetIds[i] = handle->id;
        }

        reg->compositeState->dependencyVersions =
            (uint8 *)pushAssetData(&memory->assets, sizeof(uint8) * dependencyCount);
    }
    else
    {
        reg->regType = ASSET_REG_VIRTUAL;
    }

    state->registeredAssetCount++;
    if (Platform.notifyAssetRegistered)
    {
        Platform.notifyAssetRegistered(reg);
    }

    return reg;
}

ASSETS_INITIALIZE(assetsInitialize)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;
    state->data = &memory->assets;
    state->renderCtx = rctx;
}

ASSETS_REGISTER_TEXTURE(assetsRegisterTexture)
{
    AssetRegistration *reg = registerAsset(memory, ASSET_TYPE_TEXTURE, relativePath, 0, 0);
    reg->metadata.texture = pushAssetStruct(&memory->assets, TextureAssetMetadata);
    reg->metadata.texture->is16Bit = is16Bit;
    return reg->handle;
}
ASSETS_REGISTER_SHADER(assetsRegisterShader)
{
    AssetRegistration *reg = registerAsset(memory, ASSET_TYPE_SHADER, relativePath, 0, 0);
    reg->metadata.shader = pushAssetStruct(&memory->assets, ShaderAssetMetadata);
    reg->metadata.shader->type = type;
    return reg->handle;
}
ASSETS_REGISTER_SHADER_PROGRAM(assetsRegisterShaderProgram)
{
    AssetRegistration *reg =
        registerAsset(memory, ASSET_TYPE_SHADER_PROGRAM, 0, shaderAssetHandles, shaderCount);
    return reg->handle;
}
ASSETS_REGISTER_MESH(assetsRegisterMesh)
{
    AssetRegistration *reg = registerAsset(memory, ASSET_TYPE_MESH, relativePath, 0, 0);
    return reg->handle;
}

bool buildCompositeAsset(AssetsState *state, AssetRegistration *reg, LoadedAsset **deps)
{
    uint32 assetType = ASSET_GET_TYPE(reg->handle->id);
    if (assetType == ASSET_TYPE_SHADER_PROGRAM)
    {
        uint32 shaderHandles[MAX_DEPENDENCIES_PER_ASSET];
        for (uint32 i = 0; i < reg->compositeState->dependencyCount; i++)
        {
            shaderHandles[i] = deps[i]->shader->handle;
        }

        uint32 handle;
        if (rendererCreateShaderProgram(state->renderCtx, reg->compositeState->dependencyCount,
                shaderHandles, &handle))
        {
            if (!reg->asset.shaderProgram)
            {
                reg->asset.shaderProgram = pushAssetStruct(state->data, ShaderProgramAsset);
            }
            reg->asset.shaderProgram->handle = handle;

            return true;
        }
    }
    return false;
}

LoadedAsset *getAsset(AssetsState *state, uint32 assetId)
{
    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    assert(assetIdx < state->registeredAssetCount);

    AssetRegistration *reg = &state->registeredAssets[assetIdx];
    if (reg->regType == ASSET_REG_FILE)
    {
        AssetFileState *fileState = reg->fileState;
        if (fileState->relativePath && !fileState->isUpToDate && !fileState->isLoadQueued)
        {
            if (Platform.queueAssetLoad(reg->handle, fileState->relativePath))
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
            LoadedAsset *dependency = getAsset(state, compState->dependencyAssetIds[i]);
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
            if (buildCompositeAsset(state, reg, dependencies))
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
    AssetHandleInternal *handle = (AssetHandleInternal *)assetHandle;
    assert(ASSET_GET_TYPE(handle->id) == ASSET_TYPE_SHADER);
    return getAsset(handle->state, handle->id);
}
ASSETS_GET_SHADER_PROGRAM(assetsGetShaderProgram)
{
    AssetHandleInternal *handle = (AssetHandleInternal *)assetHandle;
    assert(ASSET_GET_TYPE(handle->id) == ASSET_TYPE_SHADER_PROGRAM);
    return getAsset(handle->state, handle->id);
}
ASSETS_GET_TEXTURE(assetsGetTexture)
{
    AssetHandleInternal *handle = (AssetHandleInternal *)assetHandle;
    assert(ASSET_GET_TYPE(handle->id) == ASSET_TYPE_TEXTURE);
    return getAsset(handle->state, handle->id);
}
ASSETS_GET_MESH(assetsGetMesh)
{
    AssetHandleInternal *handle = (AssetHandleInternal *)assetHandle;
    assert(ASSET_GET_TYPE(handle->id) == ASSET_TYPE_MESH);
    return getAsset(handle->state, handle->id);
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
    MemoryBlock *memory, const char *path, void *data, uint64 size, MeshAsset *out_asset)
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

ASSETS_SET_ASSET_DATA(assetsSetAssetData)
{
    AssetHandleInternal *handle = (AssetHandleInternal *)assetHandle;
    AssetsState *state = handle->state;
    uint32 assetId = handle->id;

    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    uint32 assetType = ASSET_GET_TYPE(assetId);

    assert(assetIdx < state->registeredAssetCount);
    AssetRegistration *reg = &state->registeredAssets[assetIdx];

    if (assetType == ASSET_TYPE_SHADER)
    {
        char *src = static_cast<char *>(data);
        uint32 handle;
        if (rendererCreateShader(state->renderCtx, reg->metadata.shader->type, src, &handle))
        {
            if (!reg->asset.shader)
            {
                reg->asset.shader = pushAssetStruct(state->data, ShaderAsset);
            }
            reg->asset.shader->handle = handle;
        }
    }
    else if (assetType == ASSET_TYPE_TEXTURE)
    {
        if (!reg->asset.texture)
        {
            reg->asset.texture = pushAssetStruct(state->data, TextureAsset);
        }

        const stbi_uc *rawData = static_cast<stbi_uc *>(data);
        void *loadedData;
        int32 width;
        int32 height;
        int32 channels;
        uint64 elementSize = 0;
        if (reg->metadata.texture->is16Bit)
        {
            loadedData =
                stbi_load_16_from_memory(rawData, size, &width, &height, &channels, 0);
            elementSize = 2;
        }
        else
        {
            loadedData = stbi_load_from_memory(rawData, size, &width, &height, &channels, 0);
            elementSize = 1;
        }
        assert(width >= 0);
        assert(height >= 0);

        reg->asset.texture->width = (uint32)width;
        reg->asset.texture->height = (uint32)height;

        uint64 requiredStorage =
            (uint64)width * (uint64)height * (uint64)channels * elementSize;
        reg->asset.texture->data = (uint8 *)pushAssetData(state->data, requiredStorage);
        memcpy(reg->asset.texture->data, loadedData, requiredStorage);

        stbi_image_free(loadedData);
    }
    else if (assetType == ASSET_TYPE_MESH)
    {
        if (!reg->asset.mesh)
        {
            reg->asset.mesh = pushAssetStruct(state->data, MeshAsset);
        }
        fastObjLoadMesh(
            state->data, reg->fileState->relativePath, data, size, reg->asset.mesh);
    }
    reg->asset.version++;
    if (reg->regType == ASSET_REG_FILE)
    {
        reg->fileState->isUpToDate = true;
    }
}

ASSETS_INVALIDATE_ASSET(assetsInvalidateAsset)
{
    AssetHandleInternal *handle = (AssetHandleInternal *)assetHandle;
    AssetsState *state = handle->state;
    uint32 assetId = handle->id;

    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    assert(assetIdx < state->registeredAssetCount);
    AssetRegistration *reg = &state->registeredAssets[assetIdx];
    if (reg->regType == ASSET_REG_FILE)
    {
        reg->fileState->isUpToDate = false;
        reg->fileState->isLoadQueued = false;
    }
}