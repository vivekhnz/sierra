#ifndef ENGINE_H
#define ENGINE_H

#include "engine_assets.h"
#include "engine_heightfield.h"
#include "engine_renderer.h"

struct EngineApi
{
    AssetsInitialize *assetsInitialize;
    AssetsRegisterShader *assetsRegisterShader;
    AssetsRegisterTexture *assetsRegisterTexture;
    AssetsRegisterShaderProgram *assetsRegisterShaderProgram;
    AssetsRegisterMesh *assetsRegisterMesh;
    AssetsGetShader *assetsGetShader;
    AssetsGetShaderProgram *assetsGetShaderProgram;
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
    RendererCreateVertexArray *rendererCreateVertexArray;
    RendererBindVertexArray *rendererBindVertexArray;
    RendererUnbindVertexArray *rendererUnbindVertexArray;
    RendererCreateBuffer *rendererCreateBuffer;
    RendererBindBuffer *rendererBindBuffer;
    RendererUpdateBuffer *rendererUpdateBuffer;
    RendererBindVertexAttribute *rendererBindVertexAttribute;
    RendererCreateRenderTarget *rendererCreateRenderTarget;
    RendererResizeRenderTarget *rendererResizeRenderTarget;
    RendererCreateEffect *rendererCreateEffect;
    RendererSetEffectFloat *rendererSetEffectFloat;
    RendererSetEffectInt *rendererSetEffectInt;
    RendererSetEffectTexture *rendererSetEffectTexture;
    RendererCreateQueue *rendererCreateQueue;
    RendererSetCameraOrtho *rendererSetCameraOrtho;
    RendererSetCameraPersp *rendererSetCameraPersp;
    RendererClear *rendererClear;
    RendererPushTexturedQuad *rendererPushTexturedQuad;
    RendererPushEffectQuad *rendererPushEffectQuad;
    RendererPushEffectQuads *rendererPushEffectQuads;
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