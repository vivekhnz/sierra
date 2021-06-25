#ifndef ENGINE_H
#define ENGINE_H

#include "engine_assets.h"
#include "engine_heightfield.h"
#include "engine_renderer.h"

struct EngineApi
{
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
    RendererCreateDepthBuffer *rendererCreateDepthBuffer;
    RendererResizeDepthBuffer *rendererResizeDepthBuffer;
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
    RendererDestroyResources *rendererDestroyResources;
    RendererDrawTexturedQuad *rendererDrawTexturedQuad;
};

#define ENGINE_GET_API(name) EngineApi *name()
typedef ENGINE_GET_API(EngineGetApi);

#endif