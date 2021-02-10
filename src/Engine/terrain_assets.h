#ifndef TERRAIN_ASSETS_H
#define TERRAIN_ASSETS_H

#include "terrain_platform.h"

enum ShaderAssets
{
    ASSET_SHADER_TEXTURE_VERTEX,
    ASSET_SHADER_TEXTURE_FRAGMENT,
    ASSET_SHADER_TERRAIN_VERTEX,
    ASSET_SHADER_TERRAIN_TESS_CTRL,
    ASSET_SHADER_TERRAIN_TESS_EVAL,
    ASSET_SHADER_TERRAIN_FRAGMENT,
    ASSET_SHADER_TERRAIN_COMPUTE_TESS_LEVEL,
    ASSET_SHADER_WIREFRAME_VERTEX,
    ASSET_SHADER_WIREFRAME_TESS_CTRL,
    ASSET_SHADER_WIREFRAME_TESS_EVAL,
    ASSET_SHADER_WIREFRAME_FRAGMENT,
    ASSET_SHADER_BRUSH_VERTEX,
    ASSET_SHADER_BRUSH_FRAGMENT,

    ASSET_SHADER_COUNT
};
enum ShaderPrograms
{
    ASSET_SHADER_PROGRAM_QUAD,
    ASSET_SHADER_PROGRAM_TERRAIN_TEXTURED,
    ASSET_SHADER_PROGRAM_TERRAIN_WIREFRAME,
    ASSET_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL,
    ASSET_SHADER_PROGRAM_BRUSH,

    ASSET_SHADER_PROGRAM_COUNT
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

EXPORT ShaderAsset *assetsGetShader(EngineMemory *memory, uint32 assetId);
EXPORT ShaderProgramAsset *assetsGetShaderProgram(EngineMemory *memory, uint32 assetId);
EXPORT void assetsInvalidateShader(EngineMemory *memory, uint32 assetId);

EXPORT TextureAsset assetsLoadTexture(
    EngineMemory *memory, uint32 assetId, PlatformReadFileResult *result, bool is16Bit);

#endif