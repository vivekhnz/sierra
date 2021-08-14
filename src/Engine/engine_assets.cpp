#include "engine_assets.h"

#include <stb/stb_image.h>
#include <fast_obj/fast_obj.h>

extern EnginePlatformApi Platform;
global_variable bool WasAssetSystemReloaded = true;

#define ASSET_GET_INDEX(id) (id & 0x0FFFFFFF)
#define ASSET_GET_TYPE(id) ((id & 0xF0000000) >> 28)

#define MAX_ASSETS 4096

struct Assets
{
    MemoryArena *arena;
    RenderContext *rctx;

    AssetRegistration registeredAssets[MAX_ASSETS];
    uint32 registeredAssetCount;
};
struct AssetHandleInternal
{
    uint32 id;
    Assets *assets;
};

AssetRegistration *registerAsset(Assets *assets, AssetType type, const char *relativePath)
{
    AssetRegistration *reg = &assets->registeredAssets[assets->registeredAssetCount];
    reg->handle = pushStruct(assets->arena, AssetHandleInternal);
    reg->handle->id = assets->registeredAssetCount | (type << 28);
    reg->handle->assets = assets;
    reg->assetType = type;

    if (relativePath)
    {
        reg->regType = ASSET_REG_FILE;
        reg->fileState = pushStruct(assets->arena, AssetFileState);
        *reg->fileState = {};

        const char *srcCursor = relativePath;
        while (*srcCursor)
        {
            srcCursor++;
        }
        uint32 length = srcCursor - relativePath;
        reg->fileState->relativePath = (char *)pushSize(assets->arena, length + 1);

        srcCursor = relativePath;
        char *dstCursor = reg->fileState->relativePath;
        while (*srcCursor)
        {
            *dstCursor++ = *srcCursor++;
        }
        *dstCursor = 0;

        Platform.watchAssetFile(reg->handle, relativePath);
    }
    else
    {
        reg->regType = ASSET_REG_VIRTUAL;
    }

    assets->registeredAssetCount++;
    if (Platform.notifyAssetRegistered)
    {
        Platform.notifyAssetRegistered(reg);
    }

    return reg;
}

ASSETS_INITIALIZE(assetsInitialize)
{
    Assets *result = pushStruct(arena, Assets);
    *result = {};
    result->arena = arena;
    result->rctx = rctx;

    return result;
}

ASSETS_REGISTER_TEXTURE(assetsRegisterTexture)
{
    AssetRegistration *reg = registerAsset(assets, ASSET_TYPE_TEXTURE, relativePath);
    reg->metadata.texture = pushStruct(assets->arena, TextureAssetMetadata);
    reg->metadata.texture->format = format;
    return reg->handle;
}
ASSETS_REGISTER_SHADER(assetsRegisterShader)
{
    AssetRegistration *reg = registerAsset(assets, ASSET_TYPE_SHADER, relativePath);
    reg->metadata.shader = pushStruct(assets->arena, ShaderAssetMetadata);
    reg->metadata.shader->type = type;
    return reg->handle;
}
ASSETS_REGISTER_MESH(assetsRegisterMesh)
{
    AssetRegistration *reg = registerAsset(assets, ASSET_TYPE_MESH, relativePath);
    return reg->handle;
}

LoadedAsset *getAsset(Assets *assets, uint32 assetId)
{
    if (WasAssetSystemReloaded)
    {
        // invalidate all shaders
        for (uint32 i = 0; i < assets->registeredAssetCount; i++)
        {
            AssetRegistration *reg = &assets->registeredAssets[i];
            if (reg->assetType == ASSET_TYPE_SHADER && reg->regType == ASSET_REG_FILE)
            {
                reg->fileState->isUpToDate = false;
                reg->fileState->isLoadQueued = false;
            }
        }

        WasAssetSystemReloaded = false;
    }

    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    assert(assetIdx < assets->registeredAssetCount);

    AssetRegistration *reg = &assets->registeredAssets[assetIdx];
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

    return &reg->asset;
}

ASSETS_GET_SHADER(assetsGetShader)
{
    AssetHandleInternal *handle = (AssetHandleInternal *)assetHandle;
    assert(ASSET_GET_TYPE(handle->id) == ASSET_TYPE_SHADER);
    return getAsset(handle->assets, handle->id);
}
ASSETS_GET_TEXTURE(assetsGetTexture)
{
    AssetHandleInternal *handle = (AssetHandleInternal *)assetHandle;
    assert(ASSET_GET_TYPE(handle->id) == ASSET_TYPE_TEXTURE);
    return getAsset(handle->assets, handle->id);
}
ASSETS_GET_MESH(assetsGetMesh)
{
    AssetHandleInternal *handle = (AssetHandleInternal *)assetHandle;
    assert(ASSET_GET_TYPE(handle->id) == ASSET_TYPE_MESH);
    return getAsset(handle->assets, handle->id);
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
void fastObjLoadMesh(MemoryArena *memory, const char *path, void *data, uint64 size, MeshAsset *out_asset)
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
    out_asset->indices = (uint32 *)pushSize(memory, elementBufferSize);

    out_asset->vertexCount = out_asset->elementCount;
    uint32 vertexBufferSize = sizeof(float) * out_asset->vertexCount * 6;
    out_asset->vertices = (float *)pushSize(memory, vertexBufferSize);

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
    Assets *assets = handle->assets;
    uint32 assetId = handle->id;

    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    uint32 assetType = ASSET_GET_TYPE(assetId);

    assert(assetIdx < assets->registeredAssetCount);
    AssetRegistration *reg = &assets->registeredAssets[assetIdx];

    if (assetType == ASSET_TYPE_SHADER)
    {
        char *src = static_cast<char *>(data);
        ShaderHandle handle;
        if (createShader(assets->rctx->internalCtx, reg->metadata.shader->type, src, &handle))
        {
            if (reg->asset.shader)
            {
                destroyShader(reg->asset.shader->handle);
            }
            else
            {
                reg->asset.shader = pushStruct(assets->arena, ShaderAsset);
            }
            reg->asset.shader->handle = handle;
        }
    }
    else if (assetType == ASSET_TYPE_TEXTURE)
    {
        TextureFormat format = reg->metadata.texture->format;

        const stbi_uc *rawData = static_cast<stbi_uc *>(data);
        void *loadedData;
        int32 width;
        int32 height;
        int32 channels;
        uint64 elementSize = getTextureElementSize(format);
        if (elementSize == sizeof(uint8))
        {
            loadedData = stbi_load_from_memory(rawData, size, &width, &height, &channels, 0);
        }
        else if (elementSize == sizeof(uint16))
        {
            loadedData = stbi_load_16_from_memory(rawData, size, &width, &height, &channels, 0);
        }
        else
        {
            assert(!"Unsupported texture format");
        }
        assert(width >= 0);
        assert(height >= 0);

        uint64 requiredStorage = (uint64)width * (uint64)height * (uint64)channels * elementSize;
        void *texels = (uint8 *)pushSize(assets->arena, requiredStorage);
        memcpy(texels, loadedData, requiredStorage);

        stbi_image_free(loadedData);

        if (reg->asset.texture)
        {
            // todo: reclaim asset memory
            assert((uint32)width == reg->asset.texture->width);
            assert((uint32)height == reg->asset.texture->height);
        }
        else
        {
            reg->asset.texture = pushStruct(assets->arena, TextureAsset);
            reg->asset.texture->width = (uint32)width;
            reg->asset.texture->height = (uint32)height;
            reg->asset.texture->slot =
                reserveTextureSlot(assets->rctx->internalCtx, (uint32)width, (uint32)height, format);
        }
        reg->asset.texture->data = texels;
        updateTextureSlot(reg->asset.texture->slot, texels);
    }
    else if (assetType == ASSET_TYPE_MESH)
    {
        if (reg->asset.mesh)
        {
            destroyMesh(reg->asset.mesh->handle);
        }
        else
        {
            reg->asset.mesh = pushStruct(assets->arena, MeshAsset);
        }
        MeshAsset *mesh = reg->asset.mesh;
        fastObjLoadMesh(assets->arena, reg->fileState->relativePath, data, size, mesh);

        mesh->handle =
            createMesh(assets->arena, mesh->vertices, mesh->vertexCount, mesh->indices, mesh->elementCount);
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
    Assets *assets = handle->assets;
    uint32 assetId = handle->id;

    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    assert(assetIdx < assets->registeredAssetCount);
    AssetRegistration *reg = &assets->registeredAssets[assetIdx];
    if (reg->regType == ASSET_REG_FILE)
    {
        reg->fileState->isUpToDate = false;
        reg->fileState->isLoadQueued = false;
    }
}