#include "terrain_assets.h"

#include <glad/glad.h>
#include <stb/stb_image.h>
#include "terrain_renderer.h"

#define MAX_SHADERS_PER_PROGRAM 8

enum AssetLoadState
{
    ASSET_LOAD_STATE_UNLOADED,
    ASSET_LOAD_STATE_QUEUED,
    ASSET_LOAD_STATE_LOADED
};

struct AssetInfo
{
    AssetLoadState state;
    void *data;
};

struct ShaderInfo
{
    uint32 type;
    const char *relativePath;
};

struct AssetsState
{
    AssetInfo shaderAssetInfos[ASSET_SHADER_COUNT];
    ShaderAsset shaderAssets[ASSET_SHADER_COUNT];

    AssetInfo shaderProgramAssetInfos[ASSET_SHADER_PROGRAM_COUNT];
    ShaderProgramAsset shaderProgramAssets[ASSET_SHADER_PROGRAM_COUNT];

    uint64 dataStorageUsed;
};

ShaderInfo getShaderInfo(uint32 assetId)
{
    assert(assetId < ASSET_SHADER_COUNT);
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
    case ASSET_SHADER_BRUSH_VERTEX:
        info.type = GL_VERTEX_SHADER;
        info.relativePath = "data/brush_vertex_shader.glsl";
        break;
    case ASSET_SHADER_BRUSH_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "data/brush_fragment_shader.glsl";
        break;
    }
    return info;
}

void getShaderProgramShaders(
    uint32 assetId, uint32 *out_shaderCount, uint32 *out_shaderAssetIds)
{
    assert(assetId < ASSET_SHADER_PROGRAM_COUNT);
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
    case ASSET_SHADER_PROGRAM_BRUSH:
        *out_shaderCount = 2;
        *out_shaderAssetIds++ = ASSET_SHADER_BRUSH_VERTEX;
        *out_shaderAssetIds++ = ASSET_SHADER_BRUSH_FRAGMENT;
        break;
    }
}

void onShaderLoaded(EngineMemory *memory, uint32 assetId, PlatformReadFileResult *result)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    ShaderInfo shaderInfo = getShaderInfo(assetId);

    char *src = static_cast<char *>(result->data);
    uint32 handle;
    assert(rendererCreateShader(memory, shaderInfo.type, src, &handle));

    ShaderAsset *asset = &state->shaderAssets[assetId];
    asset->handle = handle;

    AssetInfo *assetInfo = &state->shaderAssetInfos[assetId];
    assetInfo->data = asset;
    assetInfo->state = ASSET_LOAD_STATE_LOADED;
}

ShaderAsset *assetsGetShader(EngineMemory *memory, uint32 assetId)
{
    assert(assetId < ASSET_SHADER_COUNT);
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    AssetInfo *assetInfo = &state->shaderAssetInfos[assetId];
    if (assetInfo->state != ASSET_LOAD_STATE_LOADED)
    {
        if (assetInfo->state != ASSET_LOAD_STATE_QUEUED)
        {
            ShaderInfo shaderInfo = getShaderInfo(assetId);
            if (memory->platformLoadAsset(
                    memory, assetId, shaderInfo.relativePath, onShaderLoaded))
            {
                _InterlockedCompareExchange((long *)&assetInfo->state, ASSET_LOAD_STATE_QUEUED,
                    ASSET_LOAD_STATE_UNLOADED);
            }
        }
        return 0;
    }

    return static_cast<ShaderAsset *>(assetInfo->data);
}

ShaderProgramAsset *assetsGetShaderProgram(EngineMemory *memory, uint32 assetId)
{
    assert(assetId < ASSET_SHADER_PROGRAM_COUNT);
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    AssetInfo *assetInfo = &state->shaderProgramAssetInfos[assetId];
    if (assetInfo->state != ASSET_LOAD_STATE_LOADED)
    {
        uint32 shaderCount;
        uint32 shaderAssetIds[MAX_SHADERS_PER_PROGRAM];
        getShaderProgramShaders(assetId, &shaderCount, shaderAssetIds);
        assert(shaderCount <= MAX_SHADERS_PER_PROGRAM);

        uint32 shaderHandles[MAX_SHADERS_PER_PROGRAM];
        for (uint32 i = 0; i < shaderCount; i++)
        {
            ShaderAsset *shader = assetsGetShader(memory, shaderAssetIds[i]);
            if (!shader)
            {
                return static_cast<ShaderProgramAsset *>(assetInfo->data);
            }
            shaderHandles[i] = shader->handle;
        }

        uint32 handle;
        assert(rendererCreateShaderProgram(memory, shaderCount, shaderHandles, &handle));

        ShaderProgramAsset *asset = &state->shaderProgramAssets[assetId];
        asset->handle = handle;

        assetInfo->data = asset;
        assetInfo->state = ASSET_LOAD_STATE_LOADED;
    }

    return static_cast<ShaderProgramAsset *>(assetInfo->data);
}

void assetsInvalidateShader(EngineMemory *memory, uint32 assetId)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    state->shaderAssetInfos[assetId].state = ASSET_LOAD_STATE_UNLOADED;
    for (int i = 0; i < ASSET_SHADER_PROGRAM_COUNT; i++)
    {
        uint32 shaderCount;
        uint32 shaderAssetIds[MAX_SHADERS_PER_PROGRAM];
        getShaderProgramShaders(i, &shaderCount, shaderAssetIds);

        bool isDependentOfInvalidatedShader = false;
        for (int j = 0; j < shaderCount; j++)
        {
            if (shaderAssetIds[j] == assetId)
            {
                isDependentOfInvalidatedShader = true;
                break;
            }
        }

        if (isDependentOfInvalidatedShader)
        {
            state->shaderProgramAssetInfos[i].state = ASSET_LOAD_STATE_UNLOADED;
        }
    }
}

TextureAsset assetsLoadTexture(
    EngineMemory *memory, uint32 assetId, PlatformReadFileResult *result, bool is16Bit)
{
    assert(memory->assets.size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)memory->assets.baseAddress;

    const stbi_uc *rawData = static_cast<stbi_uc *>(result->data);
    void *data;
    int32 width;
    int32 height;
    int32 channels;
    uint64 elementSize = 0;
    if (is16Bit)
    {
        data = stbi_load_16_from_memory(rawData, result->size, &width, &height, &channels, 0);
        elementSize = 2;
    }
    else
    {
        data = stbi_load_from_memory(rawData, result->size, &width, &height, &channels, 0);
        elementSize = 1;
    }

    uint64 availableStorage =
        memory->assets.size - (sizeof(AssetsState) + state->dataStorageUsed);
    uint64 requiredStorage = (uint64)width * (uint64)height * (uint64)channels * elementSize;
    assert(availableStorage >= requiredStorage);
    assert(width >= 0);
    assert(height >= 0);

    TextureAsset asset = {};
    asset.width = (uint32)width;
    asset.height = (uint32)height;
    asset.data =
        (uint8 *)memory->assets.baseAddress + sizeof(AssetsState) + state->dataStorageUsed;
    memcpy(asset.data, data, requiredStorage);
    state->dataStorageUsed += requiredStorage;

    stbi_image_free(data);

    return asset;
}