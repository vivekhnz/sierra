#include "engine.h"

#include "engine_assets.cpp"
#include "engine_heightfield.cpp"
#include "engine_renderer.cpp"

#include "../../deps/glad/glad.c"
#include "../../deps/stb/stb_image.c"
#include "../../deps/fast_obj/fast_obj.c"

global_variable EngineApi Api;
global_variable bool IsGLLoaded;
global_variable EnginePlatformApi Platform;

API_EXPORT ENGINE_GET_API(engineGetApi)
{
    Platform = platformApi;
    if (!IsGLLoaded)
    {
        bool glLoadSucceeded =
            getGlProcAddress ? gladLoadGLLoader(getGlProcAddress) : gladLoadGL();
        if (glLoadSucceeded)
        {
            IsGLLoaded = true;
        }
        else
        {
            assert(!"Failed to initialize GLAD");
        }
    }

    Api.assetsInitialize = assetsInitialize;
    Api.assetsRegisterShader = assetsRegisterShader;
    Api.assetsRegisterTexture = assetsRegisterTexture;
    Api.assetsRegisterShaderProgram = assetsRegisterShaderProgram;
    Api.assetsRegisterMesh = assetsRegisterMesh;
    Api.assetsGetShader = assetsGetShader;
    Api.assetsGetShaderProgram = assetsGetShaderProgram;
    Api.assetsGetTexture = assetsGetTexture;
    Api.assetsGetMesh = assetsGetMesh;
    Api.assetsSetAssetData = assetsSetAssetData;
    Api.assetsInvalidateAsset = assetsInvalidateAsset;

    Api.heightfieldGetHeight = heightfieldGetHeight;
    Api.heightfieldIsRayIntersecting = heightfieldIsRayIntersecting;

    Api.rendererInitialize = rendererInitialize;
    Api.rendererUpdateCameraState = rendererUpdateCameraState;
    Api.rendererUpdateLightingState = rendererUpdateLightingState;
    Api.rendererCreateTexture = rendererCreateTexture;
    Api.rendererBindTexture = rendererBindTexture;
    Api.rendererUpdateTexture = rendererUpdateTexture;
    Api.rendererReadTexturePixels = rendererReadTexturePixels;
    Api.rendererCreateTextureArray = rendererCreateTextureArray;
    Api.rendererBindTextureArray = rendererBindTextureArray;
    Api.rendererUpdateTextureArray = rendererUpdateTextureArray;
    Api.rendererCreateFramebuffer = rendererCreateFramebuffer;
    Api.rendererBindFramebuffer = rendererBindFramebuffer;
    Api.rendererUnbindFramebuffer = rendererUnbindFramebuffer;
    Api.rendererCreateShader = rendererCreateShader;
    Api.rendererCreateShaderProgram = rendererCreateShaderProgram;
    Api.rendererUseShaderProgram = rendererUseShaderProgram;
    Api.rendererSetShaderProgramUniformFloat = rendererSetShaderProgramUniformFloat;
    Api.rendererSetShaderProgramUniformInteger = rendererSetShaderProgramUniformInteger;
    Api.rendererSetShaderProgramUniformVector2 = rendererSetShaderProgramUniformVector2;
    Api.rendererSetShaderProgramUniformVector3 = rendererSetShaderProgramUniformVector3;
    Api.rendererSetShaderProgramUniformVector4 = rendererSetShaderProgramUniformVector4;
    Api.rendererSetShaderProgramUniformMatrix4x4 = rendererSetShaderProgramUniformMatrix4x4;
    Api.rendererCreateVertexArray = rendererCreateVertexArray;
    Api.rendererBindVertexArray = rendererBindVertexArray;
    Api.rendererUnbindVertexArray = rendererUnbindVertexArray;
    Api.rendererCreateBuffer = rendererCreateBuffer;
    Api.rendererBindBuffer = rendererBindBuffer;
    Api.rendererUpdateBuffer = rendererUpdateBuffer;
    Api.rendererBindVertexAttribute = rendererBindVertexAttribute;
    Api.rendererBindShaderStorageBuffer = rendererBindShaderStorageBuffer;
    Api.rendererSetViewportSize = rendererSetViewportSize;
    Api.rendererClearBackBuffer = rendererClearBackBuffer;
    Api.rendererSetPolygonMode = rendererSetPolygonMode;
    Api.rendererSetBlendMode = rendererSetBlendMode;
    Api.rendererDrawElements = rendererDrawElements;
    Api.rendererDrawElementsInstanced = rendererDrawElementsInstanced;
    Api.rendererDispatchCompute = rendererDispatchCompute;
    Api.rendererShaderStorageMemoryBarrier = rendererShaderStorageMemoryBarrier;
    Api.rendererCreateRenderTarget = rendererCreateRenderTarget;
    Api.rendererResizeRenderTarget = rendererResizeRenderTarget;
    Api.rendererCreateEffect = rendererCreateEffect;
    Api.rendererSetEffectFloat = rendererSetEffectFloat;
    Api.rendererSetEffectInt = rendererSetEffectInt;
    Api.rendererSetEffectTexture = rendererSetEffectTexture;
    Api.rendererCreateQueue = rendererCreateQueue;
    Api.rendererSetCamera = rendererSetCamera;
    Api.rendererClear = rendererClear;
    Api.rendererPushTexturedQuad = rendererPushTexturedQuad;
    Api.rendererPushEffectQuad = rendererPushEffectQuad;
    Api.rendererPushEffectQuads = rendererPushEffectQuads;
    Api.rendererDrawToTarget = rendererDrawToTarget;
    Api.rendererDrawToScreen = rendererDrawToScreen;

    return &Api;
}