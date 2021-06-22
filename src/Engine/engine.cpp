#include "engine.h"

#include "engine_assets.cpp"
#include "engine_heightfield.cpp"
#include "engine_renderer.cpp"

#include "../../deps/glad/glad.c"
#include "../../deps/stb/stb_image.c"
#include "../../deps/fast_obj/fast_obj.c"

global_variable EngineApi api;

API_EXPORT ENGINE_GET_API(engineGetApi)
{
    api.assetsRegisterShader = assetsRegisterShader;
    api.assetsRegisterTexture = assetsRegisterTexture;
    api.assetsRegisterShaderProgram = assetsRegisterShaderProgram;
    api.assetsRegisterMesh = assetsRegisterMesh;
    api.assetsGetShader = assetsGetShader;
    api.assetsGetShaderProgram = assetsGetShaderProgram;
    api.assetsGetTexture = assetsGetTexture;
    api.assetsGetMesh = assetsGetMesh;
    api.assetsSetAssetData = assetsSetAssetData;
    api.assetsInvalidateAsset = assetsInvalidateAsset;

    api.heightfieldGetHeight = heightfieldGetHeight;
    api.heightfieldIsRayIntersecting = heightfieldIsRayIntersecting;

    api.rendererInitialize = rendererInitialize;
    api.rendererUpdateCameraState = rendererUpdateCameraState;
    api.rendererUpdateLightingState = rendererUpdateLightingState;
    api.rendererCreateTexture = rendererCreateTexture;
    api.rendererBindTexture = rendererBindTexture;
    api.rendererUpdateTexture = rendererUpdateTexture;
    api.rendererReadTexturePixels = rendererReadTexturePixels;
    api.rendererCreateTextureArray = rendererCreateTextureArray;
    api.rendererBindTextureArray = rendererBindTextureArray;
    api.rendererUpdateTextureArray = rendererUpdateTextureArray;
    api.rendererCreateDepthBuffer = rendererCreateDepthBuffer;
    api.rendererResizeDepthBuffer = rendererResizeDepthBuffer;
    api.rendererCreateFramebuffer = rendererCreateFramebuffer;
    api.rendererBindFramebuffer = rendererBindFramebuffer;
    api.rendererUnbindFramebuffer = rendererUnbindFramebuffer;
    api.rendererCreateShader = rendererCreateShader;
    api.rendererCreateShaderProgram = rendererCreateShaderProgram;
    api.rendererUseShaderProgram = rendererUseShaderProgram;
    api.rendererSetShaderProgramUniformFloat = rendererSetShaderProgramUniformFloat;
    api.rendererSetShaderProgramUniformInteger = rendererSetShaderProgramUniformInteger;
    api.rendererSetShaderProgramUniformVector2 = rendererSetShaderProgramUniformVector2;
    api.rendererSetShaderProgramUniformVector3 = rendererSetShaderProgramUniformVector3;
    api.rendererSetShaderProgramUniformVector4 = rendererSetShaderProgramUniformVector4;
    api.rendererSetShaderProgramUniformMatrix4x4 = rendererSetShaderProgramUniformMatrix4x4;
    api.rendererCreateVertexArray = rendererCreateVertexArray;
    api.rendererBindVertexArray = rendererBindVertexArray;
    api.rendererUnbindVertexArray = rendererUnbindVertexArray;
    api.rendererCreateBuffer = rendererCreateBuffer;
    api.rendererBindBuffer = rendererBindBuffer;
    api.rendererUpdateBuffer = rendererUpdateBuffer;
    api.rendererBindVertexAttribute = rendererBindVertexAttribute;
    api.rendererBindShaderStorageBuffer = rendererBindShaderStorageBuffer;
    api.rendererSetViewportSize = rendererSetViewportSize;
    api.rendererClearBackBuffer = rendererClearBackBuffer;
    api.rendererSetPolygonMode = rendererSetPolygonMode;
    api.rendererSetBlendMode = rendererSetBlendMode;
    api.rendererDrawElements = rendererDrawElements;
    api.rendererDrawElementsInstanced = rendererDrawElementsInstanced;
    api.rendererDispatchCompute = rendererDispatchCompute;
    api.rendererShaderStorageMemoryBarrier = rendererShaderStorageMemoryBarrier;
    api.rendererDestroyResources = rendererDestroyResources;

    return &api;
}