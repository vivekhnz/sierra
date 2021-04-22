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

    api->rendererInitialize = rendererInitialize;
    api->rendererUpdateCameraState = rendererUpdateCameraState;
    api->rendererUpdateLightingState = rendererUpdateLightingState;
    api->rendererCreateTexture = rendererCreateTexture;
    api->rendererBindTexture = rendererBindTexture;
    api->rendererUpdateTexture = rendererUpdateTexture;
    api->rendererReadTexturePixels = rendererReadTexturePixels;
    api->rendererCreateTextureArray = rendererCreateTextureArray;
    api->rendererBindTextureArray = rendererBindTextureArray;
    api->rendererUpdateTextureArray = rendererUpdateTextureArray;
    api->rendererCreateFramebuffer = rendererCreateFramebuffer;
    api->rendererBindFramebuffer = rendererBindFramebuffer;
    api->rendererUnbindFramebuffer = rendererUnbindFramebuffer;
    api->rendererCreateShader = rendererCreateShader;
    api->rendererCreateShaderProgram = rendererCreateShaderProgram;
    api->rendererUseShaderProgram = rendererUseShaderProgram;
    api->rendererSetShaderProgramUniformFloat = rendererSetShaderProgramUniformFloat;
    api->rendererSetShaderProgramUniformInteger = rendererSetShaderProgramUniformInteger;
    api->rendererSetShaderProgramUniformVector2 = rendererSetShaderProgramUniformVector2;
    api->rendererSetShaderProgramUniformVector3 = rendererSetShaderProgramUniformVector3;
    api->rendererSetShaderProgramUniformVector4 = rendererSetShaderProgramUniformVector4;
    api->rendererSetShaderProgramUniformMatrix4x4 = rendererSetShaderProgramUniformMatrix4x4;
    api->rendererCreateVertexArray = rendererCreateVertexArray;
    api->rendererBindVertexArray = rendererBindVertexArray;
    api->rendererUnbindVertexArray = rendererUnbindVertexArray;
    api->rendererCreateBuffer = rendererCreateBuffer;
    api->rendererBindBuffer = rendererBindBuffer;
    api->rendererUpdateBuffer = rendererUpdateBuffer;
    api->rendererBindVertexAttribute = rendererBindVertexAttribute;
    api->rendererBindShaderStorageBuffer = rendererBindShaderStorageBuffer;
    api->rendererSetViewportSize = rendererSetViewportSize;
    api->rendererClearBackBuffer = rendererClearBackBuffer;
    api->rendererSetPolygonMode = rendererSetPolygonMode;
    api->rendererSetBlendMode = rendererSetBlendMode;
    api->rendererDrawElements = rendererDrawElements;
    api->rendererDrawElementsInstanced = rendererDrawElementsInstanced;
    api->rendererDispatchCompute = rendererDispatchCompute;
    api->rendererShaderStorageMemoryBarrier = rendererShaderStorageMemoryBarrier;
    api->rendererDestroyResources = rendererDestroyResources;
}
