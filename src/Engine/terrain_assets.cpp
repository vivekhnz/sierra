#include "terrain_assets.h"

#include <glad/glad.h>

#include "win32_platform.h"
#include "terrain_renderer.h"

struct AssetInfo
{
    bool isLoaded;
    void *data;
};

struct ShaderInfo
{
    uint32 type;
    const char *relativePath;
};

#define ASSET_COUNT ASSET_SHADER_COUNT

struct AssetsState
{
    AssetInfo assetInfos[ASSET_COUNT];
    ShaderAsset shaderAssets[ASSET_SHADER_COUNT];
};

AssetsState *getState(MemoryBlock *engineMemoryBlock)
{
    EngineMemory *engineMemory = static_cast<EngineMemory *>(engineMemoryBlock->baseAddress);
    MemoryBlock *assetsBlock = &engineMemory->assets;
    assert(assetsBlock->size >= sizeof(AssetsState));
    AssetsState *state = (AssetsState *)assetsBlock->baseAddress;
    return state;
}

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
    case ASSET_SHADER_UI_VERTEX:
        info.type = GL_VERTEX_SHADER;
        info.relativePath = "data/ui_vertex_shader.glsl";
        break;
    case ASSET_SHADER_UI_FRAGMENT:
        info.type = GL_FRAGMENT_SHADER;
        info.relativePath = "data/ui_fragment_shader.glsl";
        break;
    }
    return info;
}

void assetsLoadShader(MemoryBlock *memory, uint32 assetId)
{
    AssetsState *state = getState(memory);
    assert(assetId < ASSET_SHADER_COUNT);

    ShaderInfo shaderInfo = getShaderInfo(assetId);

    char absolutePath[MAX_PATH];
    win32GetAbsolutePath(shaderInfo.relativePath, absolutePath);

    Win32ReadFileResult result = win32ReadFile(absolutePath);
    assert(result.data != 0);
    char *src = static_cast<char *>(result.data);

    uint32 handle;
    assert(rendererCreateShader(memory, shaderInfo.type, src, &handle));
    win32FreeMemory(src);

    ShaderAsset *asset = &state->shaderAssets[assetId];
    asset->handle = handle;

    AssetInfo *assetInfo = &state->assetInfos[assetId];
    assetInfo->data = asset;
    assetInfo->isLoaded = true;
}

ShaderAsset *assetsGetShader(MemoryBlock *memory, uint32 assetId)
{
    AssetsState *state = getState(memory);
    assert(assetId < ASSET_SHADER_COUNT);

    AssetInfo *assetInfo = &state->assetInfos[assetId];
    if (!assetInfo->isLoaded)
    {
        assetsLoadShader(memory, assetId);
    }

    // note: we assume that the assetsLoadShader is synchronous and the assetInfo has now
    // been updated
    return static_cast<ShaderAsset *>(assetInfo->data);
}