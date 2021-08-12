#ifndef ENGINE_H
#define ENGINE_H

#include "engine_platform.h"
#include "engine_heightfield.h"
#include "engine_renderer_common.h"
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
    RendererCreateTexture *rendererCreateTexture;
    RendererUpdateTexture *rendererUpdateTexture;
    RendererGetPixels *rendererGetPixels;
    RendererGetPixelsInRegion *rendererGetPixelsInRegion;
    RendererGetTextureArray *rendererGetTextureArray;
    RendererUpdateTextureArray *rendererUpdateTextureArray;
    RendererCreateBuffer *rendererCreateBuffer;
    RendererUpdateBuffer *rendererUpdateBuffer;
    RendererCreateRenderTarget *rendererCreateRenderTarget;
    RendererResizeRenderTarget *rendererResizeRenderTarget;
    RendererCreateEffect *rendererCreateEffect;
    RendererSetEffectFloat *rendererSetEffectFloat;
    RendererSetEffectVec3 *rendererSetEffectVec3;
    RendererSetEffectInt *rendererSetEffectInt;
    RendererSetEffectUint *rendererSetEffectUint;
    RendererSetEffectTexture *rendererSetEffectTexture;
    RendererCreateQueue *rendererCreateQueue;
    RendererSetCameraOrtho *rendererSetCameraOrtho;
    RendererSetCameraOrthoOffset *rendererSetCameraOrthoOffset;
    RendererSetCameraPersp *rendererSetCameraPersp;
    RendererSetLighting *rendererSetLighting;
    RendererClear *rendererClear;
    RendererPushTexturedQuad *rendererPushTexturedQuad;
    RendererPushColoredQuad *rendererPushColoredQuad;
    RendererPushQuad *rendererPushQuad;
    RendererPushQuads *rendererPushQuads;
    RendererPushMeshes *rendererPushMeshes;
    RendererPushTerrain *rendererPushTerrain;
    RendererDrawToTarget *rendererDrawToTarget;
    RendererDrawToScreen *rendererDrawToScreen;
};

typedef void *GetGLProcAddress(const char *procName);

#define ENGINE_GET_API(name) EngineApi *name(GetGLProcAddress *getGlProcAddress, EnginePlatformApi platformApi)
typedef ENGINE_GET_API(EngineGetApi);

#endif