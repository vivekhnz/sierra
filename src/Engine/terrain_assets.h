#ifndef TERRAIN_ASSETS_H
#define TERRAIN_ASSETS_H

#include "terrain_platform.h"

enum AssetType
{
    ASSET_TYPE_SHADER = 1,
    ASSET_TYPE_SHADER_PROGRAM,
    ASSET_TYPE_TEXTURE
};

#define ASSET_ID(type, name, idx) ASSET_##type##_##name## = idx | (ASSET_TYPE_##type## << 28)

enum ShaderAssetId
{
    ASSET_ID(SHADER, TEXTURE_VERTEX, 0),
    ASSET_ID(SHADER, TEXTURE_FRAGMENT, 1),
    ASSET_ID(SHADER, TERRAIN_VERTEX, 2),
    ASSET_ID(SHADER, TERRAIN_TESS_CTRL, 3),
    ASSET_ID(SHADER, TERRAIN_TESS_EVAL, 4),
    ASSET_ID(SHADER, TERRAIN_FRAGMENT, 5),
    ASSET_ID(SHADER, TERRAIN_COMPUTE_TESS_LEVEL, 6),
    ASSET_ID(SHADER, WIREFRAME_VERTEX, 7),
    ASSET_ID(SHADER, WIREFRAME_TESS_CTRL, 8),
    ASSET_ID(SHADER, WIREFRAME_TESS_EVAL, 9),
    ASSET_ID(SHADER, WIREFRAME_FRAGMENT, 10),
    ASSET_ID(SHADER, BRUSH_MASK_VERTEX, 11),
    ASSET_ID(SHADER, BRUSH_MASK_FRAGMENT, 12),
    ASSET_ID(SHADER, BRUSH_BLEND_ADD_SUB_FRAGMENT, 13),
    ASSET_ID(SHADER, BRUSH_BLEND_FLATTEN_FRAGMENT, 14)
};
#define ASSET_SHADER_COUNT 15

enum ShaderProgramAssetId
{
    ASSET_ID(SHADER_PROGRAM, QUAD, 0),
    ASSET_ID(SHADER_PROGRAM, TERRAIN_TEXTURED, 1),
    ASSET_ID(SHADER_PROGRAM, TERRAIN_WIREFRAME, 2),
    ASSET_ID(SHADER_PROGRAM, TERRAIN_CALC_TESS_LEVEL, 3),
    ASSET_ID(SHADER_PROGRAM, BRUSH_MASK, 4),
    ASSET_ID(SHADER_PROGRAM, BRUSH_BLEND_ADD_SUB, 5),
    ASSET_ID(SHADER_PROGRAM, BRUSH_BLEND_FLATTEN, 6)
};
#define ASSET_SHADER_PROGRAM_COUNT 7

enum TextureAssetId
{
    ASSET_ID(TEXTURE, GROUND_ALBEDO, 0),
    ASSET_ID(TEXTURE, GROUND_NORMAL, 1),
    ASSET_ID(TEXTURE, GROUND_DISPLACEMENT, 2),
    ASSET_ID(TEXTURE, GROUND_AO, 3),
    ASSET_ID(TEXTURE, ROCK_ALBEDO, 4),
    ASSET_ID(TEXTURE, ROCK_NORMAL, 5),
    ASSET_ID(TEXTURE, ROCK_DISPLACEMENT, 6),
    ASSET_ID(TEXTURE, ROCK_AO, 7),
    ASSET_ID(TEXTURE, SNOW_ALBEDO, 8),
    ASSET_ID(TEXTURE, SNOW_NORMAL, 9),
    ASSET_ID(TEXTURE, SNOW_DISPLACEMENT, 10),
    ASSET_ID(TEXTURE, SNOW_AO, 11)
};
#define ASSET_TEXTURE_COUNT 12

struct ShaderAsset
{
    uint32 handle;
    uint8 version;
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
    uint8 version;
};

EXPORT ShaderAsset *assetsGetShader(EngineMemory *memory, uint32 assetId);
EXPORT ShaderProgramAsset *assetsGetShaderProgram(EngineMemory *memory, uint32 assetId);

EXPORT void assetsLoadTexture(
    EngineMemory *memory, void *data, uint64 size, bool is16Bit, TextureAsset *out_asset);
EXPORT TextureAsset *assetsGetTexture(EngineMemory *memory, uint32 assetId);

EXPORT void assetsInvalidateAsset(EngineMemory *memory, uint32 assetId);

#endif