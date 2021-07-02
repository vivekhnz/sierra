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
    RendererUpdateCameraState *rendererUpdateCameraState;
    RendererUpdateLightingState *rendererUpdateLightingState;
    RendererCreateTexture *rendererCreateTexture;
    RendererBindTexture *rendererBindTexture;
    RendererUpdateTexture *rendererUpdateTexture;
    RendererReadTexturePixels *rendererReadTexturePixels;
    RendererCreateTextureArray *rendererCreateTextureArray;
    RendererBindTextureArray *rendererBindTextureArray;
    RendererUpdateTextureArray *rendererUpdateTextureArray;
    RendererCreateFramebuffer *rendererCreateFramebuffer;
    RendererBindFramebuffer *rendererBindFramebuffer;
    RendererUnbindFramebuffer *rendererUnbindFramebuffer;
    RendererCreateShader *rendererCreateShader;
    RendererCreateShaderProgram *rendererCreateShaderProgram;
    RendererUseShaderProgram *rendererUseShaderProgram;
    RendererSetShaderProgramUniformFloat *rendererSetShaderProgramUniformFloat;
    RendererSetShaderProgramUniformInteger *rendererSetShaderProgramUniformInteger;
    RendererSetShaderProgramUniformVector2 *rendererSetShaderProgramUniformVector2;
    RendererSetShaderProgramUniformVector3 *rendererSetShaderProgramUniformVector3;
    RendererSetShaderProgramUniformVector4 *rendererSetShaderProgramUniformVector4;
    RendererSetShaderProgramUniformMatrix4x4 *rendererSetShaderProgramUniformMatrix4x4;
    RendererCreateVertexArray *rendererCreateVertexArray;
    RendererBindVertexArray *rendererBindVertexArray;
    RendererUnbindVertexArray *rendererUnbindVertexArray;
    RendererCreateBuffer *rendererCreateBuffer;
    RendererBindBuffer *rendererBindBuffer;
    RendererUpdateBuffer *rendererUpdateBuffer;
    RendererBindVertexAttribute *rendererBindVertexAttribute;
    RendererBindShaderStorageBuffer *rendererBindShaderStorageBuffer;
    RendererSetViewportSize *rendererSetViewportSize;
    RendererClearBackBuffer *rendererClearBackBuffer;
    RendererSetPolygonMode *rendererSetPolygonMode;
    RendererSetBlendMode *rendererSetBlendMode;
    RendererDrawElements *rendererDrawElements;
    RendererDrawElementsInstanced *rendererDrawElementsInstanced;
    RendererDispatchCompute *rendererDispatchCompute;
    RendererShaderStorageMemoryBarrier *rendererShaderStorageMemoryBarrier;
    RendererCreateRenderTarget *rendererCreateRenderTarget;
    RendererResizeRenderTarget *rendererResizeRenderTarget;
    RendererCreateEffect *rendererCreateEffect;
    RendererSetEffectFloat *rendererSetEffectFloat;
    RendererSetEffectInt *rendererSetEffectInt;
    RendererSetEffectTexture *rendererSetEffectTexture;
    RendererCreateQueue *rendererCreateQueue;
    RendererSetCamera *rendererSetCamera;
    RendererClear *rendererClear;
    RendererPushTexturedQuad *rendererPushTexturedQuad;
    RendererPushEffectQuad *rendererPushEffectQuad;
    RendererPushEffectQuads *rendererPushEffectQuads;
    RendererDrawToTarget *rendererDrawToTarget;
    RendererDrawToScreen *rendererDrawToScreen;
};

typedef void *GetGLProcAddress(const char *procName);

#define ENGINE_GET_API(name)                                                                  \
    EngineApi *name(GetGLProcAddress *getGlProcAddress, EnginePlatformApi platformApi)
typedef ENGINE_GET_API(EngineGetApi);

#endif