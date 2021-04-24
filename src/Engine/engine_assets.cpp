#include "engine_assets.h"

#include <stb/stb_image.h>
#include <fast_obj/fast_obj.h>
#include "engine_renderer.h"

#define ASSET_GET_INDEX(id) (id & 0x0FFFFFFF)
#define ASSET_GET_TYPE(id) ((id & 0xF0000000) >> 28)

#define MAX_SHADERS_PER_PROGRAM 8

RENDERER_CREATE_SHADER_PROGRAM(rendererCreateShaderProgram);
RENDERER_CREATE_SHADER(rendererCreateShader);

struct ShaderAssetSlot
{
    bool isUpToDate;
    bool isLoadQueued;
    ShaderAsset *asset;
};
struct ShaderProgramAssetSlot
{
    ShaderProgramAsset *asset;
    uint8 shaderVersions[MAX_SHADERS_PER_PROGRAM];
};
struct TextureAssetSlot
{
    bool isUpToDate;
    bool isLoadQueued;
    TextureAsset *asset;
};
struct MeshAssetSlot
{
    bool isUpToDate;
    bool isLoadQueued;
    MeshAsset *asset;
};

struct ShaderInfo
{
    uint32 type;
    const char *relativePath;
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
    ShaderAssetSlot shaderAssetSlots[ASSET_SHADER_COUNT];
    ShaderAsset shaderAssets[ASSET_SHADER_COUNT];

    ShaderProgramAssetSlot shaderProgramAssetSlot[ASSET_SHADER_PROGRAM_COUNT];
    ShaderProgramAsset shaderProgramAssets[ASSET_SHADER_PROGRAM_COUNT];

    TextureAssetSlot textureAssetSlots[ASSET_TEXTURE_COUNT];
    TextureAsset textureAssets[ASSET_TEXTURE_COUNT];

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

ShaderInfo getShaderInfo(uint32 assetId)
{
    ShaderInfo info = {};
    switch (assetId)
    {
    case ASSET_SHADER_TEXTURE_VERTEX:
        info.type = GL_VERTEX_SHADER;
        info.relativePath = "texture_vertex_shader.glsl";
        break;
    case ASSET_SHADER_TEXTURE_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "texture_fragment_shader.glsl";
        break;
    case ASSET_SHADER_TERRAIN_VERTEX:
        info.type = GL_VERTEX_SHADER;
        info.relativePath = "terrain_vertex_shader.glsl";
        break;
    case ASSET_SHADER_TERRAIN_TESS_CTRL:
        info.type = GL_TESS_CONTROL_SHADER;
        info.relativePath = "terrain_tess_ctrl_shader.glsl";
        break;
    case ASSET_SHADER_TERRAIN_TESS_EVAL:
        info.type = GL_TESS_EVALUATION_SHADER;
        info.relativePath = "terrain_tess_eval_shader.glsl";
        break;
    case ASSET_SHADER_TERRAIN_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "terrain_fragment_shader.glsl";
        break;
    case ASSET_SHADER_TERRAIN_COMPUTE_TESS_LEVEL:
        info.type = GL_COMPUTE_SHADER;
        info.relativePath = "terrain_calc_tess_levels_comp_shader.glsl";
        break;
    case ASSET_SHADER_WIREFRAME_VERTEX:
        info.type = GL_VERTEX_SHADER;
        info.relativePath = "wireframe_vertex_shader.glsl";
        break;
    case ASSET_SHADER_WIREFRAME_TESS_CTRL:
        info.type = GL_TESS_CONTROL_SHADER;
        info.relativePath = "wireframe_tess_ctrl_shader.glsl";
        break;
    case ASSET_SHADER_WIREFRAME_TESS_EVAL:
        info.type = GL_TESS_EVALUATION_SHADER;
        info.relativePath = "wireframe_tess_eval_shader.glsl";
        break;
    case ASSET_SHADER_WIREFRAME_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "wireframe_fragment_shader.glsl";
        break;
    case ASSET_SHADER_BRUSH_MASK_VERTEX:
        info.type = GL_VERTEX_SHADER;
        info.relativePath = "brush_mask_vertex_shader.glsl";
        break;
    case ASSET_SHADER_BRUSH_MASK_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "brush_mask_fragment_shader.glsl";
        break;
    case ASSET_SHADER_BRUSH_BLEND_ADD_SUB_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "brush_blend_add_sub_fragment_shader.glsl";
        break;
    case ASSET_SHADER_BRUSH_BLEND_FLATTEN_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "brush_blend_flatten_fragment_shader.glsl";
        break;
    case ASSET_SHADER_BRUSH_BLEND_SMOOTH_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "brush_blend_smooth_fragment_shader.glsl";
        break;
    case ASSET_SHADER_ROCK_VERTEX:
        info.type = GL_VERTEX_SHADER;
        info.relativePath = "rock_vertex_shader.glsl";
        break;
    case ASSET_SHADER_ROCK_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "rock_fragment_shader.glsl";
        break;
    }
    return info;
}

void getShaderProgramShaders(
    uint32 assetId, uint32 *out_shaderCount, uint32 *out_shaderAssetIds)
{
    switch (assetId)
    {
    case ASSET_SHADER_PROGRAM_QUAD:
        *out_shaderCount = 2;
        *out_shaderAssetIds++ = ASSET_SHADER_TEXTURE_VERTEX;
        *out_shaderAssetIds++ = ASSET_SHADER_TEXTURE_FRAGMENT;
        break;
    case ASSET_SHADER_PROGRAM_TERRAIN_TEXTURED:
        *out_shaderCount = 4;
        *out_shaderAssetIds++ = ASSET_SHADER_TERRAIN_VERTEX;
        *out_shaderAssetIds++ = ASSET_SHADER_TERRAIN_TESS_CTRL;
        *out_shaderAssetIds++ = ASSET_SHADER_TERRAIN_TESS_EVAL;
        *out_shaderAssetIds++ = ASSET_SHADER_TERRAIN_FRAGMENT;
        break;
    case ASSET_SHADER_PROGRAM_TERRAIN_WIREFRAME:
        *out_shaderCount = 4;
        *out_shaderAssetIds++ = ASSET_SHADER_WIREFRAME_VERTEX;
        *out_shaderAssetIds++ = ASSET_SHADER_WIREFRAME_TESS_CTRL;
        *out_shaderAssetIds++ = ASSET_SHADER_WIREFRAME_TESS_EVAL;
        *out_shaderAssetIds++ = ASSET_SHADER_WIREFRAME_FRAGMENT;
        break;
    case ASSET_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL:
        *out_shaderCount = 1;
        *out_shaderAssetIds++ = ASSET_SHADER_TERRAIN_COMPUTE_TESS_LEVEL;
        break;
    case ASSET_SHADER_PROGRAM_BRUSH_MASK:
        *out_shaderCount = 2;
        *out_shaderAssetIds++ = ASSET_SHADER_BRUSH_MASK_VERTEX;
        *out_shaderAssetIds++ = ASSET_SHADER_BRUSH_MASK_FRAGMENT;
        break;
    case ASSET_SHADER_PROGRAM_BRUSH_BLEND_ADD_SUB:
        *out_shaderCount = 2;
        *out_shaderAssetIds++ = ASSET_SHADER_TEXTURE_VERTEX;
        *out_shaderAssetIds++ = ASSET_SHADER_BRUSH_BLEND_ADD_SUB_FRAGMENT;
        break;
    case ASSET_SHADER_PROGRAM_BRUSH_BLEND_FLATTEN:
        *out_shaderCount = 2;
        *out_shaderAssetIds++ = ASSET_SHADER_TEXTURE_VERTEX;
        *out_shaderAssetIds++ = ASSET_SHADER_BRUSH_BLEND_FLATTEN_FRAGMENT;
        break;
    case ASSET_SHADER_PROGRAM_BRUSH_BLEND_SMOOTH:
        *out_shaderCount = 2;
        *out_shaderAssetIds++ = ASSET_SHADER_TEXTURE_VERTEX;
        *out_shaderAssetIds++ = ASSET_SHADER_BRUSH_BLEND_SMOOTH_FRAGMENT;
        break;
    case ASSET_SHADER_PROGRAM_ROCK:
        *out_shaderCount = 2;
        *out_shaderAssetIds++ = ASSET_SHADER_ROCK_VERTEX;
        *out_shaderAssetIds++ = ASSET_SHADER_ROCK_FRAGMENT;
        break;
    }
}

TextureInfo getTextureInfo(uint32 assetId)
{
    TextureInfo info = {};
    switch (assetId)
    {
    case ASSET_TEXTURE_GROUND_ALBEDO:
        info.relativePath = "ground_albedo.bmp";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_GROUND_NORMAL:
        info.relativePath = "ground_normal.bmp";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_GROUND_DISPLACEMENT:
        info.relativePath = "ground_displacement.tga";
        info.is16Bit = true;
        break;
    case ASSET_TEXTURE_GROUND_AO:
        info.relativePath = "ground_ao.tga";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_ROCK_ALBEDO:
        info.relativePath = "rock_albedo.jpg";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_ROCK_NORMAL:
        info.relativePath = "rock_normal.jpg";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_ROCK_DISPLACEMENT:
        info.relativePath = "rock_displacement.tga";
        info.is16Bit = true;
        break;
    case ASSET_TEXTURE_ROCK_AO:
        info.relativePath = "rock_ao.tga";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_SNOW_ALBEDO:
        info.relativePath = "snow_albedo.jpg";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_SNOW_NORMAL:
        info.relativePath = "snow_normal.jpg";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_SNOW_DISPLACEMENT:
        info.relativePath = "snow_displacement.tga";
        info.is16Bit = true;
        break;
    case ASSET_TEXTURE_SNOW_AO:
        info.relativePath = "snow_ao.tga";
        info.is16Bit = false;
        break;
    }
    return info;
}

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

ASSETS_GET_REGISTERED_TEXTURE_ASSETS(assetsGetRegisteredTextureAssets)
{
    TextureAssetRegistration *assets = (TextureAssetRegistration *)pushAssetData(
        memory, sizeof(TextureAssetRegistration) * ASSET_TEXTURE_COUNT);
    *out_count = ASSET_TEXTURE_COUNT;

    for (uint32 i = 0; i < ASSET_TEXTURE_COUNT; i++)
    {
        TextureAssetRegistration *reg = &assets[i];

        reg->id = i | (ASSET_TYPE_TEXTURE << 28);
        TextureInfo info = getTextureInfo(reg->id);

        const char *srcCursor = info.relativePath;
        char *dstCursor = reg->relativePath;
        while (*srcCursor)
        {
            *dstCursor++ = *srcCursor++;
        }
    }

    return assets;
}

ASSETS_GET_SHADER(assetsGetShader)
{
    assert(ASSET_GET_TYPE(assetId) == ASSET_TYPE_SHADER);
    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    assert(assetIdx < ASSET_SHADER_COUNT);
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    ShaderAssetSlot *slot = &state->shaderAssetSlots[assetIdx];
    if (!slot->isUpToDate && !slot->isLoadQueued)
    {
        ShaderInfo shaderInfo = getShaderInfo(assetId);
        if (memory->platformLoadAsset(assetId, shaderInfo.relativePath))
        {
            slot->isLoadQueued = true;
        }
    }

    return slot->asset;
}

ASSETS_GET_SHADER_PROGRAM(assetsGetShaderProgram)
{
    assert(ASSET_GET_TYPE(assetId) == ASSET_TYPE_SHADER_PROGRAM);
    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    assert(assetIdx < ASSET_SHADER_PROGRAM_COUNT);
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    ShaderProgramAssetSlot *slot = &state->shaderProgramAssetSlot[assetIdx];

    uint32 shaderCount;
    uint32 shaderAssetIds[MAX_SHADERS_PER_PROGRAM];
    getShaderProgramShaders(assetId, &shaderCount, shaderAssetIds);
    assert(shaderCount <= MAX_SHADERS_PER_PROGRAM);

    uint32 shaderHandles[MAX_SHADERS_PER_PROGRAM];
    uint8 shaderVersions[MAX_SHADERS_PER_PROGRAM];
    bool shouldCreateShaderProgram = false;
    for (uint32 i = 0; i < shaderCount; i++)
    {
        ShaderAsset *shader = assetsGetShader(memory, shaderAssetIds[i]);
        if (!shader)
        {
            shouldCreateShaderProgram = false;
            break;
        }
        if (shader && shader->version > slot->shaderVersions[i])
        {
            shouldCreateShaderProgram = true;
        }
        shaderHandles[i] = shader->handle;
        shaderVersions[i] = shader->version;
    }

    if (shouldCreateShaderProgram)
    {
        uint32 handle;
        if (rendererCreateShaderProgram(memory, shaderCount, shaderHandles, &handle))
        {
            ShaderProgramAsset *asset = &state->shaderProgramAssets[assetIdx];
            asset->handle = handle;

            slot->asset = asset;
            for (uint32 i = 0; i < shaderCount; i++)
            {
                slot->shaderVersions[i] = shaderVersions[i];
            }
        }
    }

    return slot->asset;
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
    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    assert(assetIdx < ASSET_TEXTURE_COUNT);

    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    TextureAssetSlot *slot = &state->textureAssetSlots[assetIdx];
    if (!slot->isUpToDate && !slot->isLoadQueued)
    {
        TextureInfo textureInfo = getTextureInfo(assetId);
        if (memory->platformLoadAsset(assetId, textureInfo.relativePath))
        {
            slot->isLoadQueued = true;
        }
    }

    return slot->asset;
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

    if (assetType == ASSET_TYPE_SHADER)
    {
        assert(assetIdx < ASSET_SHADER_COUNT);
        ShaderInfo shaderInfo = getShaderInfo(assetId);

        char *src = static_cast<char *>(data);
        uint32 handle;
        if (rendererCreateShader(memory, shaderInfo.type, src, &handle))
        {
            ShaderAsset *asset = &state->shaderAssets[assetIdx];
            asset->version++;
            asset->handle = handle;

            ShaderAssetSlot *slot = &state->shaderAssetSlots[assetIdx];
            slot->asset = asset;
            slot->isUpToDate = true;
        }
    }
    else if (assetType == ASSET_TYPE_TEXTURE)
    {
        assert(assetIdx < ASSET_TEXTURE_COUNT);
        TextureInfo textureInfo = getTextureInfo(assetId);

        TextureAsset *asset = &state->textureAssets[assetIdx];
        assetsLoadTexture(memory, data, size, textureInfo.is16Bit, asset);
        asset->version++;

        TextureAssetSlot *slot = &state->textureAssetSlots[assetIdx];
        slot->asset = asset;
        slot->isUpToDate = true;
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

    if (assetType == ASSET_TYPE_SHADER)
    {
        assert(assetIdx < ASSET_SHADER_COUNT);
        ShaderAssetSlot *slot = &state->shaderAssetSlots[assetIdx];
        slot->isUpToDate = false;
        slot->isLoadQueued = false;
    }
    else if (assetType == ASSET_TYPE_TEXTURE)
    {
        assert(assetIdx < ASSET_TEXTURE_COUNT);
        TextureAssetSlot *slot = &state->textureAssetSlots[assetIdx];
        slot->isUpToDate = false;
        slot->isLoadQueued = false;
    }
    else if (assetType == ASSET_TYPE_MESH)
    {
        assert(assetIdx < ASSET_MESH_COUNT);
        MeshAssetSlot *slot = &state->meshAssetSlots[assetIdx];
        slot->isUpToDate = false;
        slot->isLoadQueued = false;
    }
}