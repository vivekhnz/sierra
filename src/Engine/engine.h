#ifndef ENGINE_H
#define ENGINE_H

#include "engine_platform.h"
#include "engine_heightfield.h"
#include "engine_renderer.h"
#include "engine_assets.h"

struct EngineApi
{
    AssetsInitialize *assetsInitialize;
    AssetsRegisterShader *assetsRegisterShader;
    AssetsRegisterTexture *assetsRegisterTexture;
    AssetsRegisterMesh *assetsRegisterMesh;
    AssetsGetShader *assetsGetShader;
    AssetsGetTexture *assetsGetTexture;
    AssetsGetMesh *assetsGetMesh;
    AssetsSetAssetData *assetsSetAssetData;
    AssetsInvalidateAsset *assetsInvalidateAsset;

    HeightfieldGetHeight *heightfieldGetHeight;
    HeightfieldIsRayIntersecting *heightfieldIsRayIntersecting;

    RendererInitialize *rendererInitialize;
    RendererUpdateLightingState *rendererUpdateLightingState;
    RendererCreateTexture *rendererCreateTexture;
    RendererUpdateTexture *rendererUpdateTexture;
    RendererReadTexturePixels *rendererReadTexturePixels;
    RendererCreateTextureArray *rendererCreateTextureArray;
    RendererUpdateTextureArray *rendererUpdateTextureArray;
    RendererCreateBuffer *rendererCreateBuffer;
    RendererUpdateBuffer *rendererUpdateBuffer;
    RendererCreateRenderTarget *rendererCreateRenderTarget;
    RendererResizeRenderTarget *rendererResizeRenderTarget;
    RendererCreateEffect *rendererCreateEffect;
    RendererSetEffectFloat *rendererSetEffectFloat;
    RendererSetEffectInt *rendererSetEffectInt;
    RendererSetEffectUint *rendererSetEffectUint;
    RendererSetEffectTexture *rendererSetEffectTexture;
    RendererCreateQueue *rendererCreateQueue;
    RendererSetCameraOrtho *rendererSetCameraOrtho;
    RendererSetCameraPersp *rendererSetCameraPersp;
    RendererClear *rendererClear;
    RendererPushTexturedQuad *rendererPushTexturedQuad;
    RendererPushQuad *rendererPushQuad;
    RendererPushQuads *rendererPushQuads;
    RendererPushMeshes *rendererPushMeshes;
    RendererPushTerrain *rendererPushTerrain;
    RendererDrawToTarget *rendererDrawToTarget;
    RendererDrawToScreen *rendererDrawToScreen;
};

typedef void *GetGLProcAddress(const char *procName);

#define ENGINE_GET_API(name)                                                                  \
    EngineApi *name(GetGLProcAddress *getGlProcAddress, EnginePlatformApi platformApi)
typedef ENGINE_GET_API(EngineGetApi);

#endif