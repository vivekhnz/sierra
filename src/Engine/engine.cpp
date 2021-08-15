#include "engine.h"

#include "engine_heightfield.cpp"
#include "engine_renderer.cpp"
#include "engine_render_backend_opengl.cpp"
#include "engine_assets.cpp"

#include "../../deps/stb/stb_image.c"
#include "../../deps/fast_obj/fast_obj.c"

global_variable EngineApi Api;
global_variable bool WasEngineReloaded = true;
global_variable EnginePlatformApi Platform;

API_EXPORT ENGINE_GET_API(engineGetApi)
{
    Platform = platformApi;
    if (WasEngineReloaded)
    {
        RenderBackendInitParams initParams;
        initParams.getGlProcAddress = getGlProcAddress;
        reloadRenderBackend(initParams);

        WasEngineReloaded = false;
    }

    Api.assetsInitialize = assetsInitialize;
    Api.assetsRegisterShader = assetsRegisterShader;
    Api.assetsRegisterTexture = assetsRegisterTexture;
    Api.assetsRegisterMesh = assetsRegisterMesh;
    Api.assetsGetShader = assetsGetShader;
    Api.assetsGetTexture = assetsGetTexture;
    Api.assetsGetMesh = assetsGetMesh;
    Api.assetsSetAssetData = assetsSetAssetData;
    Api.assetsInvalidateAsset = assetsInvalidateAsset;

    Api.heightfieldGetHeight = heightfieldGetHeight;
    Api.heightfieldIsRayIntersecting = heightfieldIsRayIntersecting;

    Api.rendererInitialize = rendererInitialize;
    Api.rendererCreateTexture = rendererCreateTexture;
    Api.rendererUpdateTexture = rendererUpdateTexture;
    Api.rendererGetPixels = rendererGetPixels;
    Api.rendererGetPixelsInRegion = rendererGetPixelsInRegion;
    Api.rendererCreateRenderTarget = rendererCreateRenderTarget;
    Api.rendererResizeRenderTarget = rendererResizeRenderTarget;
    Api.rendererCreateEffect = rendererCreateEffect;
    Api.rendererSetEffectFloat = rendererSetEffectFloat;
    Api.rendererSetEffectVec3 = rendererSetEffectVec3;
    Api.rendererSetEffectInt = rendererSetEffectInt;
    Api.rendererSetEffectUint = rendererSetEffectUint;
    Api.rendererSetEffectTexture = rendererSetEffectTexture;
    Api.rendererCreateQueue = rendererCreateQueue;
    Api.rendererSetCameraOrtho = rendererSetCameraOrtho;
    Api.rendererSetCameraOrthoOffset = rendererSetCameraOrthoOffset;
    Api.rendererSetCameraPersp = rendererSetCameraPersp;
    Api.rendererSetLighting = rendererSetLighting;
    Api.rendererClear = rendererClear;
    Api.rendererPushTexturedQuad = rendererPushTexturedQuad;
    Api.rendererPushColoredQuad = rendererPushColoredQuad;
    Api.rendererPushQuad = rendererPushQuad;
    Api.rendererPushQuads = rendererPushQuads;
    Api.rendererPushMeshes = rendererPushMeshes;
    Api.rendererPushTerrain = rendererPushTerrain;
    Api.rendererDrawToTarget = rendererDrawToTarget;
    Api.rendererDrawToScreen = rendererDrawToScreen;

    return &Api;
}