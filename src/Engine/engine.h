#ifndef ENGINE_H
#define ENGINE_H

#include "engine_platform.h"
#include "engine_heightfield.h"
#include "engine_renderer_common.h"
#include "engine_renderer.h"
#include "engine_assets.h"

typedef ASSETS_INITIALIZE(AssetsInitialize);
typedef ASSETS_REGISTER_TEXTURE(AssetsRegisterTexture);
typedef ASSETS_REGISTER_SHADER(AssetsRegisterShader);
typedef ASSETS_REGISTER_MESH(AssetsRegisterMesh);
typedef ASSETS_GET_SHADER(AssetsGetShader);
typedef ASSETS_GET_TEXTURE(AssetsGetTexture);
typedef ASSETS_GET_MESH(AssetsGetMesh);
typedef ASSETS_SET_ASSET_DATA(AssetsSetAssetData);
typedef ASSETS_INVALIDATE_ASSET(AssetsInvalidateAsset);

typedef HEIGHTFIELD_GET_HEIGHT(HeightfieldGetHeight);
typedef HEIGHTFIELD_IS_RAY_INTERSECTING(HeightfieldIsRayIntersecting);

typedef RENDERER_INITIALIZE(RendererInitialize);
typedef RENDERER_CREATE_TEXTURE(RendererCreateTexture);
typedef RENDERER_UPDATE_TEXTURE(RendererUpdateTexture);
typedef RENDERER_GET_PIXELS(RendererGetPixels);
typedef RENDERER_GET_PIXELS_IN_REGION(RendererGetPixelsInRegion);
typedef RENDERER_CREATE_RENDER_TARGET(RendererCreateRenderTarget);
typedef RENDERER_RESIZE_RENDER_TARGET(RendererResizeRenderTarget);
typedef RENDERER_CREATE_EFFECT(RendererCreateEffect);
typedef RENDERER_SET_EFFECT_FLOAT(RendererSetEffectFloat);
typedef RENDERER_SET_EFFECT_VEC3(RendererSetEffectVec3);
typedef RENDERER_SET_EFFECT_INT(RendererSetEffectInt);
typedef RENDERER_SET_EFFECT_UINT(RendererSetEffectUint);
typedef RENDERER_SET_EFFECT_TEXTURE(RendererSetEffectTexture);
typedef RENDERER_CREATE_QUEUE(RendererCreateQueue);
typedef RENDERER_SET_CAMERA_ORTHO(RendererSetCameraOrtho);
typedef RENDERER_SET_CAMERA_ORTHO_OFFSET(RendererSetCameraOrthoOffset);
typedef RENDERER_SET_CAMERA_PERSP(RendererSetCameraPersp);
typedef RENDERER_SET_LIGHTING(RendererSetLighting);
typedef RENDERER_CLEAR(RendererClear);
typedef RENDERER_PUSH_TEXTURED_QUAD(RendererPushTexturedQuad);
typedef RENDERER_PUSH_COLORED_QUAD(RendererPushColoredQuad);
typedef RENDERER_PUSH_QUAD(RendererPushQuad);
typedef RENDERER_PUSH_QUADS(RendererPushQuads);
typedef RENDERER_PUSH_MESHES(RendererPushMeshes);
typedef RENDERER_PUSH_TERRAIN(RendererPushTerrain);
typedef RENDERER_DRAW_TO_TARGET(RendererDrawToTarget);
typedef RENDERER_DRAW_TO_SCREEN(RendererDrawToScreen);

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