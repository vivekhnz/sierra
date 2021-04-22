#ifndef ENGINE_H
#define ENGINE_H

#include "engine_assets.h"
#include "engine_heightfield.h"
#include "engine_renderer.h"

struct EnginePlatformApi
{
    AssetsOnAssetLoaded *assetsOnAssetLoaded;
    AssetsInvalidateAsset *assetsInvalidateAsset;
    AssetsLoadTexture *assetsLoadTexture;
};

struct EngineClientApi
{
    AssetsGetShader *assetsGetShader;
    AssetsGetShaderProgram *assetsGetShaderProgram;
    AssetsLoadTexture *assetsLoadTexture;
    AssetsGetTexture *assetsGetTexture;
    AssetsGetMesh *assetsGetMesh;

    HeightfieldGetHeight *heightfieldGetHeight;
    HeightfieldIsRayIntersecting *heightfieldIsRayIntersecting;
};

EXPORT void engineGetPlatformApi(EnginePlatformApi *api);
EXPORT void engineGetClientApi(EngineClientApi *api);

#endif