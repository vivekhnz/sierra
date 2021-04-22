#include "engine.h"

#include "engine_assets.cpp"
#include "engine_heightfield.cpp"
#include "engine_renderer.cpp"

void engineGetPlatformApi(EnginePlatformApi *api)
{
    api->assetsOnAssetLoaded = assetsOnAssetLoaded;
    api->assetsInvalidateAsset = assetsInvalidateAsset;
    api->assetsLoadTexture = assetsLoadTexture;
}

void engineGetClientApi(EngineClientApi *api)
{
    api->assetsGetShader = assetsGetShader;
    api->assetsGetShaderProgram = assetsGetShaderProgram;
    api->assetsLoadTexture = assetsLoadTexture;
    api->assetsGetTexture = assetsGetTexture;
    api->assetsGetMesh = assetsGetMesh;

    api->heightfieldGetHeight = heightfieldGetHeight;
    api->heightfieldIsRayIntersecting = heightfieldIsRayIntersecting;
}
