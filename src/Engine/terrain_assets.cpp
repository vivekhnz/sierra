#include "terrain_assets.h"

#include <stb/stb_image.h>
#include <fast_obj/fast_obj.h>
#include "terrain_renderer.h"

#define ASSET_GET_INDEX(id) (id & 0x0FFFFFFF)
#define ASSET_GET_TYPE(id) ((id & 0xF0000000) >> 28)

#define MAX_SHADERS_PER_PROGRAM 8

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
        info.relativePath = "data/texture_vertex_shader.glsl";
        break;
    case ASSET_SHADER_TEXTURE_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "data/texture_fragment_shader.glsl";
        break;
    case ASSET_SHADER_TERRAIN_VERTEX:
        info.type = GL_VERTEX_SHADER;
        info.relativePath = "data/terrain_vertex_shader.glsl";
        break;
    case ASSET_SHADER_TERRAIN_TESS_CTRL:
        info.type = GL_TESS_CONTROL_SHADER;
        info.relativePath = "data/terrain_tess_ctrl_shader.glsl";
        break;
    case ASSET_SHADER_TERRAIN_TESS_EVAL:
        info.type = GL_TESS_EVALUATION_SHADER;
        info.relativePath = "data/terrain_tess_eval_shader.glsl";
        break;
    case ASSET_SHADER_TERRAIN_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "data/terrain_fragment_shader.glsl";
        break;
    case ASSET_SHADER_TERRAIN_COMPUTE_TESS_LEVEL:
        info.type = GL_COMPUTE_SHADER;
        info.relativePath = "data/terrain_calc_tess_levels_comp_shader.glsl";
        break;
    case ASSET_SHADER_WIREFRAME_VERTEX:
        info.type = GL_VERTEX_SHADER;
        info.relativePath = "data/wireframe_vertex_shader.glsl";
        break;
    case ASSET_SHADER_WIREFRAME_TESS_CTRL:
        info.type = GL_TESS_CONTROL_SHADER;
        info.relativePath = "data/wireframe_tess_ctrl_shader.glsl";
        break;
    case ASSET_SHADER_WIREFRAME_TESS_EVAL:
        info.type = GL_TESS_EVALUATION_SHADER;
        info.relativePath = "data/wireframe_tess_eval_shader.glsl";
        break;
    case ASSET_SHADER_WIREFRAME_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "data/wireframe_fragment_shader.glsl";
        break;
    case ASSET_SHADER_BRUSH_MASK_VERTEX:
        info.type = GL_VERTEX_SHADER;
        info.relativePath = "data/brush_mask_vertex_shader.glsl";
        break;
    case ASSET_SHADER_BRUSH_MASK_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "data/brush_mask_fragment_shader.glsl";
        break;
    case ASSET_SHADER_BRUSH_BLEND_ADD_SUB_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "data/brush_blend_add_sub_fragment_shader.glsl";
        break;
    case ASSET_SHADER_BRUSH_BLEND_FLATTEN_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "data/brush_blend_flatten_fragment_shader.glsl";
        break;
    case ASSET_SHADER_BRUSH_BLEND_SMOOTH_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "data/brush_blend_smooth_fragment_shader.glsl";
        break;
    case ASSET_SHADER_ROCK_VERTEX:
        info.type = GL_VERTEX_SHADER;
        info.relativePath = "data/rock_vertex_shader.glsl";
        break;
    case ASSET_SHADER_ROCK_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "data/rock_fragment_shader.glsl";
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
        info.relativePath = "data/ground_albedo.bmp";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_GROUND_NORMAL:
        info.relativePath = "data/ground_normal.bmp";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_GROUND_DISPLACEMENT:
        info.relativePath = "data/ground_displacement.tga";
        info.is16Bit = true;
        break;
    case ASSET_TEXTURE_GROUND_AO:
        info.relativePath = "data/ground_ao.tga";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_ROCK_ALBEDO:
        info.relativePath = "data/rock_albedo.jpg";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_ROCK_NORMAL:
        info.relativePath = "data/rock_normal.jpg";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_ROCK_DISPLACEMENT:
        info.relativePath = "data/rock_displacement.tga";
        info.is16Bit = true;
        break;
    case ASSET_TEXTURE_ROCK_AO:
        info.relativePath = "data/rock_ao.tga";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_SNOW_ALBEDO:
        info.relativePath = "data/snow_albedo.jpg";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_SNOW_NORMAL:
        info.relativePath = "data/snow_normal.jpg";
        info.is16Bit = false;
        break;
    case ASSET_TEXTURE_SNOW_DISPLACEMENT:
        info.relativePath = "data/snow_displacement.tga";
        info.is16Bit = true;
        break;
    case ASSET_TEXTURE_SNOW_AO:
        info.relativePath = "data/snow_ao.tga";
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
        info.relativePath = "data/rock.obj";
        break;
    }
    return info;
}

PLATFORM_ASSET_LOAD_CALLBACK(onShaderLoaded)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    ShaderInfo shaderInfo = getShaderInfo(assetId);

    char *src = static_cast<char *>(data);
    uint32 handle;
    if (rendererCreateShader(memory, shaderInfo.type, src, &handle))
    {
        uint32 assetIdx = ASSET_GET_INDEX(assetId);
        assert(assetIdx < ASSET_SHADER_COUNT);

        ShaderAsset *asset = &state->shaderAssets[assetIdx];
        asset->version++;
        asset->handle = handle;

        ShaderAssetSlot *slot = &state->shaderAssetSlots[assetIdx];
        slot->asset = asset;
        slot->isUpToDate = true;
    }
}

ShaderAsset *assetsGetShader(EngineMemory *memory, uint32 assetId)
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
        if (memory->platformLoadAsset(assetId, shaderInfo.relativePath, onShaderLoaded))
        {
            slot->isLoadQueued = true;
        }
    }

    return slot->asset;
}

ShaderProgramAsset *assetsGetShaderProgram(EngineMemory *memory, uint32 assetId)
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
    bool shouldCreateShader = false;
    for (uint32 i = 0; i < shaderCount; i++)
    {
        ShaderAsset *shader = assetsGetShader(memory, shaderAssetIds[i]);
        if (!shader)
        {
            shouldCreateShader = false;
            break;
        }
        if (shader && shader->version > slot->shaderVersions[i])
        {
            shouldCreateShader = true;
        }
        shaderHandles[i] = shader->handle;
        shaderVersions[i] = shader->version;
    }

    if (shouldCreateShader)
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

EXPORT void assetsLoadTexture(
    EngineMemory *memory, void *data, uint64 size, bool is16Bit, TextureAsset *out_asset)
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

PLATFORM_ASSET_LOAD_CALLBACK(onTextureLoaded)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    assert(assetIdx < ASSET_TEXTURE_COUNT);

    TextureInfo textureInfo = getTextureInfo(assetId);

    TextureAsset *asset = &state->textureAssets[assetIdx];
    assetsLoadTexture(memory, data, size, textureInfo.is16Bit, asset);
    asset->version++;

    TextureAssetSlot *slot = &state->textureAssetSlots[assetIdx];
    slot->asset = asset;
    slot->isUpToDate = true;
}

TextureAsset *assetsGetTexture(EngineMemory *memory, uint32 assetId)
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
        if (memory->platformLoadAsset(assetId, textureInfo.relativePath, onTextureLoaded))
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
void *fast_obj_file_open(const char *path, void *user_data)
{
    return user_data;
}
void fast_obj_file_close(void *file, void *user_data)
{
    FastObjVirtualFile *virtualFile = (FastObjVirtualFile *)file;
    virtualFile->position = 0;
}
uint64 fast_obj_file_read(void *file, void *dst, uint64 bytes, void *user_data)
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
unsigned long fast_obj_file_size(void *file, void *user_data)
{
    FastObjVirtualFile *virtualFile = (FastObjVirtualFile *)file;
    return virtualFile->size;
}

PLATFORM_ASSET_LOAD_CALLBACK(onMeshLoaded)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    uint32 assetIdx = ASSET_GET_INDEX(assetId);
    assert(assetIdx < ASSET_MESH_COUNT);

    MeshInfo meshInfo = getMeshInfo(assetId);
    MeshAsset *asset = &state->meshAssets[assetIdx];

    fastObjCallbacks callbacks = {};
    callbacks.file_open = fast_obj_file_open;
    callbacks.file_close = fast_obj_file_close;
    callbacks.file_read = fast_obj_file_read;
    callbacks.file_size = fast_obj_file_size;

    FastObjVirtualFile virtualFile = {};
    virtualFile.data = data;
    virtualFile.size = size;

    fastObjMesh *mesh =
        fast_obj_read_with_callbacks(meshInfo.relativePath, &callbacks, &virtualFile);

    asset->elementCount = mesh->face_count * 3;
    uint32 elementBufferSize = sizeof(uint32) * asset->elementCount;
    asset->indices = (uint32 *)pushAssetData(memory, elementBufferSize);

    asset->vertexCount = asset->elementCount;
    uint32 vertexBufferSize = sizeof(float) * asset->vertexCount * 6;
    asset->vertices = (float *)pushAssetData(memory, vertexBufferSize);

    uint32 *currentIndex = (uint32 *)asset->indices;
    float *currentVertex = (float *)asset->vertices;
    for (uint32 i = 0; i < asset->elementCount; i++)
    {
        fastObjIndex index = mesh->indices[i];
        *currentIndex++ = i;
        *currentVertex++ = mesh->positions[index.p * 3];
        *currentVertex++ = mesh->positions[(index.p * 3) + 1];
        *currentVertex++ = mesh->positions[(index.p * 3) + 2];
        *currentVertex++ = mesh->normals[index.n * 3] * -1;
        *currentVertex++ = mesh->normals[(index.n * 3) + 1];
        *currentVertex++ = mesh->normals[(index.n * 3) + 2];
    }

    fast_obj_destroy(mesh);

    asset->version++;

    MeshAssetSlot *slot = &state->meshAssetSlots[assetIdx];
    slot->asset = asset;
    slot->isUpToDate = true;
}

MeshAsset *assetsGetMesh(EngineMemory *memory, uint32 assetId)
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
        if (memory->platformLoadAsset(assetId, meshInfo.relativePath, onMeshLoaded))
        {
            slot->isLoadQueued = true;
        }
    }

    return slot->asset;
}

void assetsInvalidateAsset(EngineMemory *memory, uint32 assetId)
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
}