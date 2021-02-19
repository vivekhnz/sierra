#include <iostream>

#include "../Engine/terrain_assets.h"
#include "../Engine/terrain_renderer.h"
#include "../Engine/terrain_heightfield.h"
#include "../Engine/Graphics/GlfwManager.hpp"
#include "../Engine/Graphics/Window.hpp"
#include "../Engine/EngineContext.hpp"
#include "../Engine/World.hpp"
#include "../Engine/IO/Path.hpp"
#include "../Engine/IO/MouseInputState.hpp"
#include "GameContext.hpp"
#include "terrain_platform_win32.h"

void reloadHeightmap(Terrain::Engine::EngineContext *ctx,
    Heightfield *heightfield,
    uint32 textureHandle,
    const char *relativePath)
{
    PlatformReadFileResult result = ctx->memory->platformReadFile(
        Terrain::Engine::IO::Path::getAbsolutePath(relativePath).c_str());
    assert(result.data);

    TextureAsset asset;
    assetsLoadTexture(ctx->memory, &result, true, &asset);
    rendererUpdateTexture(ctx->memory, textureHandle, GL_UNSIGNED_SHORT, GL_R16, GL_RED,
        asset.width, asset.height, asset.data);

    uint16 heightmapWidth = 2048;
    uint16 heightmapHeight = 2048;
    uint16 patchTexelWidth = heightmapWidth / heightfield->columns;
    uint16 patchTexelHeight = heightmapHeight / heightfield->rows;

    uint16 *src = (uint16 *)asset.data;
    float *dst = (float *)heightfield->heights;
    float heightScalar = heightfield->maxHeight / (float)UINT16_MAX;
    for (uint32 y = 0; y < heightfield->rows; y++)
    {
        for (uint32 x = 0; x < heightfield->columns; x++)
        {
            *dst++ = *src * heightScalar;
            src += patchTexelWidth;
        }
        src += (patchTexelHeight - 1) * heightmapWidth;
    }

    ctx->memory->platformFreeMemory(result.data);
}

int main()
{
    try
    {
#define ENGINE_MEMORY_SIZE (500 * 1024 * 1024)
        EngineMemory memory;
        memory.baseAddress = win32AllocateMemory(ENGINE_MEMORY_SIZE);
        memory.size = ENGINE_MEMORY_SIZE;
        memory.platformFreeMemory = win32FreeMemory;
        memory.platformReadFile = win32ReadFile;
        memory.platformLoadAsset = win32LoadAsset;

        Terrain::Engine::Graphics::GlfwManager glfw;
        Terrain::Engine::Graphics::Window window(glfw, 1280, 720, "Terrain", false);
        window.makePrimary();
        GameContext appCtx(window);
        Terrain::Engine::EngineContext ctx(appCtx, &memory);
        ctx.initialize();
        ctx.input.addInputController();

        Terrain::Engine::World world(ctx);

// create terrain
#define HEIGHTFIELD_ROWS 256
#define HEIGHTFIELD_COLUMNS 256

        float heightfieldHeights[HEIGHTFIELD_ROWS * HEIGHTFIELD_COLUMNS] = {0};

        Heightfield heightfield = {};
        heightfield.columns = HEIGHTFIELD_COLUMNS;
        heightfield.rows = HEIGHTFIELD_ROWS;
        heightfield.spacing = 0.5f;
        heightfield.maxHeight = 25.0f;
        heightfield.heights = heightfieldHeights;
        heightfield.position = glm::vec2(-63.75f, -63.75f);

        uint32 tessLevelBufferHandle =
            rendererCreateBuffer(&memory, RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
        rendererUpdateBuffer(&memory, tessLevelBufferHandle,
            heightfield.columns * heightfield.rows * sizeof(glm::vec4), 0);

        int terrainEntityId = ctx.entities.create();
        int terrainRendererInstanceId = world.componentManagers.terrainRenderer.create(
            terrainEntityId, heightfield.rows, heightfield.columns, heightfield.spacing);

        uint32 heightmapTextureHandle = rendererCreateTexture(&memory, GL_UNSIGNED_SHORT,
            GL_R16, GL_RED, 2048, 2048, GL_MIRRORED_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        reloadHeightmap(&ctx, &heightfield, heightmapTextureHandle, "data/heightmap.tga");

        uint32 albedoTextureArrayHandle = rendererCreateTextureArray(&memory, GL_UNSIGNED_BYTE,
            GL_RGB, GL_RGB, 2048, 2048, 3, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        uint32 normalTextureArrayHandle = rendererCreateTextureArray(&memory, GL_UNSIGNED_BYTE,
            GL_RGB, GL_RGB, 2048, 2048, 3, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        uint32 aoTextureArrayHandle = rendererCreateTextureArray(&memory, GL_UNSIGNED_BYTE,
            GL_R8, GL_RED, 2048, 2048, 3, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

        uint32 groundDisplacementTextureHandle = rendererCreateTexture(&memory,
            GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048, 2048, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        uint32 rockDisplacementTextureHandle = rendererCreateTexture(&memory, GL_UNSIGNED_BYTE,
            GL_RGB, GL_RED, 2048, 2048, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        uint32 snowDisplacementTextureHandle = rendererCreateTexture(&memory, GL_UNSIGNED_BYTE,
            GL_RGB, GL_RED, 2048, 2048, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

        uint8 groundAlbedoTextureVersion = 0;
        uint8 rockAlbedoTextureVersion = 0;
        uint8 snowAlbedoTextureVersion = 0;
        uint8 groundNormalTextureVersion = 0;
        uint8 rockNormalTextureVersion = 0;
        uint8 snowNormalTextureVersion = 0;
        uint8 groundDisplacementTextureVersion = 0;
        uint8 rockDisplacementTextureVersion = 0;
        uint8 snowDisplacementTextureVersion = 0;
        uint8 groundAoTextureVersion = 0;
        uint8 rockAoTextureVersion = 0;
        uint8 snowAoTextureVersion = 0;

        // first person camera state
        float firstPersonCameraYaw = -1.57f;
        float firstPersonCameraPitch = 0.0f;
        glm::vec3 firstPersonCameraPos = glm::vec3(0, 4, 50);
        glm::vec3 firstPersonCameraLookAt = glm::vec3(0, 4, 49);

        // orbit camera state
        float orbitCameraDistance = 112.5f;
        glm::vec3 orbitCameraPos = glm::vec3(0);
        glm::vec3 orbitCameraLookAt = glm::vec3(0);
        float orbitCameraPitch = glm::radians(15.0f);
        float orbitCameraYaw = glm::radians(90.0f);

        float now = 0;
        float lastTickTime = glfw.getCurrentTime();
        float deltaTime = 0;

        bool isOrbitCameraMode = false;
        bool isWireframeMode = false;

        glm::vec4 lightDir = glm::vec4(-0.588f, 0.809f, 0.294f, 0.0f);
        bool isLightingEnabled = true;
        bool isAlbedoEnabled = true;
        bool isNormalMapEnabled = true;
        bool isAOMapEnabled = true;
        bool isDisplacementMapEnabled = true;

        bool isLightingStateUpdated = false;

        while (!window.isRequestingClose())
        {
            win32LoadQueuedAssets(&memory);

            // query input
            ctx.input.update();

            if (ctx.input.isKeyDown(0, Terrain::Engine::IO::Key::Escape))
            {
                window.close();
            }

            // swap camera mode when C key is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::C))
            {
                isOrbitCameraMode = !isOrbitCameraMode;
            }

            // toggle lighting when L key is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::L))
            {
                isLightingEnabled = !isLightingEnabled;
                isLightingStateUpdated = true;
            }

            // toggle albedo texture when T key is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::T))
            {
                isAlbedoEnabled = !isAlbedoEnabled;
                isLightingStateUpdated = true;
            }

            // toggle normal map texture when N key is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::N))
            {
                isNormalMapEnabled = !isNormalMapEnabled;
                isLightingStateUpdated = true;
            }

            // toggle displacement map texture when B key is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::B))
            {
                isDisplacementMapEnabled = !isDisplacementMapEnabled;
                isLightingStateUpdated = true;
            }

            // toggle ambient occlusion texture when O key is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::O))
            {
                isAOMapEnabled = !isAOMapEnabled;
                isLightingStateUpdated = true;
            }

            // load a different heightmap when H is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::H))
            {
                reloadHeightmap(
                    &ctx, &heightfield, heightmapTextureHandle, "data/heightmap2.tga");
            }

            // toggle terrain wireframe mode when Z is pressed
            if (ctx.input.isNewKeyPress(0, Terrain::Engine::IO::Key::Z))
            {
                isWireframeMode = !isWireframeMode;
            }

            if (isLightingStateUpdated)
            {
                rendererUpdateLightingState(&memory, &lightDir, isLightingEnabled,
                    isAlbedoEnabled, isNormalMapEnabled, isAOMapEnabled,
                    isDisplacementMapEnabled);
                isLightingStateUpdated = false;
            }

            // update world
            now = glfw.getCurrentTime();
            deltaTime = now - lastTickTime;
            lastTickTime = now;

            if (isOrbitCameraMode)
            {
                bool isManipulatingOrbitCamera = false;
                const Terrain::Engine::IO::MouseInputState &mouseState =
                    ctx.input.getMouseState(0);

                // orbit distance is modified by scrolling the mouse wheel
                orbitCameraDistance *= 1.0f - (glm::sign(mouseState.scrollOffsetY) * 0.05f);

                // only update the look at position if the middle mouse button is pressed
                if (ctx.input.isMouseButtonDown(0, Terrain::Engine::IO::MouseButton::Middle))
                {
                    glm::vec3 lookDir = glm::normalize(orbitCameraLookAt - orbitCameraPos);
                    glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
                    glm::vec3 yDir = cross(lookDir, xDir);
                    glm::vec3 pan =
                        (xDir * mouseState.cursorOffsetX) + (yDir * mouseState.cursorOffsetY);
                    orbitCameraLookAt +=
                        pan * min(max(orbitCameraDistance, 2.5f), 300.0f) * 0.02f * deltaTime;

                    isManipulatingOrbitCamera = true;
                }

                // only update yaw & pitch if the right mouse button is pressed
                if (ctx.input.isMouseButtonDown(0, Terrain::Engine::IO::MouseButton::Right))
                {
                    float rotateSensitivity =
                        0.05f * min(max(orbitCameraDistance, 14.0f), 70.0f) * deltaTime;
                    orbitCameraYaw +=
                        glm::radians(mouseState.cursorOffsetX * rotateSensitivity);
                    orbitCameraPitch +=
                        glm::radians(mouseState.cursorOffsetY * rotateSensitivity);

                    isManipulatingOrbitCamera = true;
                }

                // calculate camera position
                glm::vec3 newLookDir = glm::vec3(cos(orbitCameraYaw) * cos(orbitCameraPitch),
                    sin(orbitCameraPitch), sin(orbitCameraYaw) * cos(orbitCameraPitch));
                orbitCameraPos = orbitCameraLookAt + (newLookDir * orbitCameraDistance);

                // capture mouse if orbit camera is being manipulated
                if (isManipulatingOrbitCamera)
                {
                    ctx.input.captureMouse(false);
                }
            }
            else
            {
                const float lookSensitivity = 0.07f * deltaTime;
                const float moveSpeed = 4.0 * deltaTime;
                const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

                const Terrain::Engine::IO::MouseInputState &mouseState =
                    ctx.input.getMouseState(0);

                // rotate camera by moving mouse cursor
                firstPersonCameraYaw += mouseState.cursorOffsetX * lookSensitivity;
                firstPersonCameraPitch =
                    min(max(firstPersonCameraPitch
                                - ((float)mouseState.cursorOffsetY * lookSensitivity),
                            -1.55f),
                        1.55f);
                glm::vec3 lookDir =
                    glm::vec3(cos(firstPersonCameraYaw) * cos(firstPersonCameraPitch),
                        sin(firstPersonCameraPitch),
                        sin(firstPersonCameraYaw) * cos(firstPersonCameraPitch));

                // move camera on XZ axis using WASD keys
                glm::vec3 moveDir =
                    glm::vec3(cos(firstPersonCameraYaw), 0.0f, sin(firstPersonCameraYaw));
                if (ctx.input.isKeyDown(0, Terrain::Engine::IO::Key::A))
                {
                    firstPersonCameraPos -=
                        glm::normalize(glm::cross(moveDir, up)) * moveSpeed;
                }
                if (ctx.input.isKeyDown(0, Terrain::Engine::IO::Key::D))
                {
                    firstPersonCameraPos +=
                        glm::normalize(glm::cross(moveDir, up)) * moveSpeed;
                }
                if (ctx.input.isKeyDown(0, Terrain::Engine::IO::Key::W))
                {
                    firstPersonCameraPos += moveDir * moveSpeed;
                }
                if (ctx.input.isKeyDown(0, Terrain::Engine::IO::Key::S))
                {
                    firstPersonCameraPos -= moveDir * moveSpeed;
                }

                // smoothly lerp Y to terrain height
                float targetHeight = heightfieldGetHeight(&heightfield, firstPersonCameraPos.x,
                                         firstPersonCameraPos.z)
                    + 1.75f;
                firstPersonCameraPos.y =
                    (firstPersonCameraPos.y * 0.95f) + (targetHeight * 0.05f);

                firstPersonCameraLookAt = firstPersonCameraPos + lookDir;

                // capture mouse if first person camera is active
                ctx.input.captureMouse(false);
            }

            // render world
            Terrain::Engine::EngineViewContext vctx = appCtx.getViewContext();

            constexpr float fov = glm::pi<float>() / 4.0f;
            const float nearPlane = 0.1f;
            const float farPlane = 10000.0f;
            const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
            const float aspectRatio = (float)vctx.viewportWidth / (float)vctx.viewportHeight;

            glm::vec3 *cameraPos = isOrbitCameraMode ? &orbitCameraPos : &firstPersonCameraPos;
            glm::vec3 *cameraLookAt =
                isOrbitCameraMode ? &orbitCameraLookAt : &firstPersonCameraLookAt;
            glm::mat4 cameraTransform = glm::perspective(fov, aspectRatio, nearPlane, farPlane)
                * glm::lookAt(*cameraPos, *cameraLookAt, up);

            rendererUpdateCameraState(&memory, &cameraTransform);
            rendererSetViewportSize(vctx.viewportWidth, vctx.viewportHeight);
            rendererClearBackBuffer(0.392f, 0.584f, 0.929f, 1);

            TextureAsset *asset;
            asset = assetsGetTexture(&memory, ASSET_TEXTURE_GROUND_ALBEDO);
            if (asset && asset->version > groundAlbedoTextureVersion)
            {
                rendererUpdateTextureArray(&memory, albedoTextureArrayHandle, GL_UNSIGNED_BYTE,
                    GL_RGB, asset->width, asset->height, 0, asset->data);
                groundAlbedoTextureVersion = asset->version;
            }
            asset = assetsGetTexture(&memory, ASSET_TEXTURE_GROUND_NORMAL);
            if (asset && asset->version > groundNormalTextureVersion)
            {
                rendererUpdateTextureArray(&memory, normalTextureArrayHandle, GL_UNSIGNED_BYTE,
                    GL_RGB, asset->width, asset->height, 0, asset->data);
                groundNormalTextureVersion = asset->version;
            }
            asset = assetsGetTexture(&memory, ASSET_TEXTURE_GROUND_DISPLACEMENT);
            if (asset && asset->version > groundDisplacementTextureVersion)
            {
                rendererUpdateTexture(&memory, groundDisplacementTextureHandle,
                    GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048, 2048, asset->data);
                groundDisplacementTextureVersion = asset->version;
            }
            asset = assetsGetTexture(&memory, ASSET_TEXTURE_GROUND_AO);
            if (asset && asset->version > groundAoTextureVersion)
            {
                rendererUpdateTextureArray(&memory, aoTextureArrayHandle, GL_UNSIGNED_BYTE,
                    GL_RED, asset->width, asset->height, 0, asset->data);
                groundAoTextureVersion = asset->version;
            }
            asset = assetsGetTexture(&memory, ASSET_TEXTURE_ROCK_ALBEDO);
            if (asset && asset->version > rockAlbedoTextureVersion)
            {
                rendererUpdateTextureArray(&memory, albedoTextureArrayHandle, GL_UNSIGNED_BYTE,
                    GL_RGB, asset->width, asset->height, 1, asset->data);
                rockAlbedoTextureVersion = asset->version;
            }
            asset = assetsGetTexture(&memory, ASSET_TEXTURE_ROCK_NORMAL);
            if (asset && asset->version > rockNormalTextureVersion)
            {
                rendererUpdateTextureArray(&memory, normalTextureArrayHandle, GL_UNSIGNED_BYTE,
                    GL_RGB, asset->width, asset->height, 1, asset->data);
                rockNormalTextureVersion = asset->version;
            }
            asset = assetsGetTexture(&memory, ASSET_TEXTURE_ROCK_DISPLACEMENT);
            if (asset && asset->version > rockDisplacementTextureVersion)
            {
                rendererUpdateTexture(&memory, rockDisplacementTextureHandle, GL_UNSIGNED_BYTE,
                    GL_RGB, GL_RED, 2048, 2048, asset->data);
                rockDisplacementTextureVersion = asset->version;
            }
            asset = assetsGetTexture(&memory, ASSET_TEXTURE_ROCK_AO);
            if (asset && asset->version > rockAoTextureVersion)
            {
                rendererUpdateTextureArray(&memory, aoTextureArrayHandle, GL_UNSIGNED_BYTE,
                    GL_RED, asset->width, asset->height, 1, asset->data);
                rockAoTextureVersion = asset->version;
            }
            asset = assetsGetTexture(&memory, ASSET_TEXTURE_SNOW_ALBEDO);
            if (asset && asset->version > snowAlbedoTextureVersion)
            {
                rendererUpdateTextureArray(&memory, albedoTextureArrayHandle, GL_UNSIGNED_BYTE,
                    GL_RGB, asset->width, asset->height, 2, asset->data);
                snowAlbedoTextureVersion = asset->version;
            }
            asset = assetsGetTexture(&memory, ASSET_TEXTURE_SNOW_NORMAL);
            if (asset && asset->version > snowNormalTextureVersion)
            {
                rendererUpdateTextureArray(&memory, normalTextureArrayHandle, GL_UNSIGNED_BYTE,
                    GL_RGB, asset->width, asset->height, 2, asset->data);
                snowNormalTextureVersion = asset->version;
            }
            asset = assetsGetTexture(&memory, ASSET_TEXTURE_SNOW_DISPLACEMENT);
            if (asset && asset->version > snowDisplacementTextureVersion)
            {
                rendererUpdateTexture(&memory, snowDisplacementTextureHandle, GL_UNSIGNED_BYTE,
                    GL_RGB, GL_RED, 2048, 2048, asset->data);
                snowDisplacementTextureVersion = asset->version;
            }
            asset = assetsGetTexture(&memory, ASSET_TEXTURE_SNOW_AO);
            if (asset && asset->version > snowAoTextureVersion)
            {
                rendererUpdateTextureArray(&memory, aoTextureArrayHandle, GL_UNSIGNED_BYTE,
                    GL_RED, asset->width, asset->height, 2, asset->data);
                snowAoTextureVersion = asset->version;
            }

            uint32 terrainShaderProgramAssetId = isWireframeMode
                ? ASSET_SHADER_PROGRAM_TERRAIN_WIREFRAME
                : ASSET_SHADER_PROGRAM_TERRAIN_TEXTURED;
            uint32 terrainPolygonMode = isWireframeMode ? GL_LINE : GL_FILL;

            ShaderProgramAsset *calcTessLevelShaderProgram =
                assetsGetShaderProgram(&memory, ASSET_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL);
            ShaderProgramAsset *terrainShaderProgram =
                assetsGetShaderProgram(&memory, terrainShaderProgramAssetId);
            if (calcTessLevelShaderProgram && terrainShaderProgram)
            {
                uint32 meshHandle = world.componentManagers.terrainRenderer.getMeshHandle(
                    terrainRendererInstanceId);
                uint32 meshVertexBufferHandle =
                    ctx.assets.graphics.getMeshVertexBufferHandle(meshHandle, 0);
                uint32 meshVertexArrayHandle =
                    ctx.assets.graphics.getMeshVertexArrayHandle(meshHandle);
                uint32 meshEdgeCount = (2 * (heightfield.rows * heightfield.columns))
                    - heightfield.rows - heightfield.columns;
                uint32 elementCount = ctx.assets.graphics.getMeshElementCount(meshHandle);
                uint32 primitiveType = ctx.assets.graphics.getMeshPrimitiveType(meshHandle);

                rendererSetShaderProgramUniformFloat(
                    &memory, calcTessLevelShaderProgram->handle, "targetTriangleSize", 0.015f);
                rendererSetShaderProgramUniformInteger(&memory,
                    calcTessLevelShaderProgram->handle, "horizontalEdgeCount",
                    heightfield.rows * (heightfield.columns - 1));
                rendererSetShaderProgramUniformInteger(&memory,
                    calcTessLevelShaderProgram->handle, "columnCount", heightfield.columns);
                rendererSetShaderProgramUniformFloat(&memory,
                    calcTessLevelShaderProgram->handle, "heightfield.maxHeight",
                    heightfield.maxHeight);
                rendererBindTexture(&memory, heightmapTextureHandle, 0);
                rendererBindShaderStorageBuffer(&memory, tessLevelBufferHandle, 0);
                rendererBindShaderStorageBuffer(&memory, meshVertexBufferHandle, 1);
                rendererUseShaderProgram(&memory, calcTessLevelShaderProgram->handle);
                rendererDispatchCompute(meshEdgeCount, 1, 1);
                rendererShaderStorageMemoryBarrier();

                rendererUseShaderProgram(&memory, terrainShaderProgram->handle);
                rendererSetPolygonMode(terrainPolygonMode);
                rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                rendererSetShaderProgramUniformVector2(&memory, terrainShaderProgram->handle,
                    "mat1_textureSizeInWorldUnits", glm::vec2(2.5f, 2.5f));
                rendererSetShaderProgramUniformVector2(&memory, terrainShaderProgram->handle,
                    "mat2_textureSizeInWorldUnits", glm::vec2(13.0f, 13.0f));
                rendererSetShaderProgramUniformVector4(&memory, terrainShaderProgram->handle,
                    "mat2_rampParams", glm::vec4(0.6f, 0.8f, 0, 0.001f));
                rendererSetShaderProgramUniformVector2(&memory, terrainShaderProgram->handle,
                    "mat3_textureSizeInWorldUnits", glm::vec2(2.0f, 2.0f));
                rendererSetShaderProgramUniformVector4(&memory, terrainShaderProgram->handle,
                    "mat3_rampParams", glm::vec4(0.8f, 0.75f, 0.25f, 0.28f));
                rendererSetShaderProgramUniformVector2(&memory, terrainShaderProgram->handle,
                    "brushHighlightPos", glm::vec2(0.0f, 0.0f));
                rendererSetShaderProgramUniformFloat(
                    &memory, terrainShaderProgram->handle, "brushHighlightStrength", 0.0f);
                rendererSetShaderProgramUniformFloat(
                    &memory, terrainShaderProgram->handle, "brushHighlightRadius", 0.0f);
                rendererSetShaderProgramUniformFloat(
                    &memory, terrainShaderProgram->handle, "brushHighlightFalloff", 0.0f);
                rendererSetShaderProgramUniformVector3(
                    &memory, terrainShaderProgram->handle, "color", glm::vec3(0, 1, 0));
                rendererSetShaderProgramUniformVector3(&memory, terrainShaderProgram->handle,
                    "terrainDimensions",
                    glm::vec3(heightfield.spacing * heightfield.columns, heightfield.maxHeight,
                        heightfield.spacing * heightfield.rows));
                rendererBindTexture(&memory, heightmapTextureHandle, 0);
                rendererBindTextureArray(&memory, albedoTextureArrayHandle, 1);
                rendererBindTextureArray(&memory, normalTextureArrayHandle, 2);
                rendererBindTextureArray(&memory, aoTextureArrayHandle, 4);
                rendererBindTexture(&memory, groundDisplacementTextureHandle, 3);
                rendererBindTexture(&memory, rockDisplacementTextureHandle, 7);
                rendererBindTexture(&memory, snowDisplacementTextureHandle, 11);
                rendererBindVertexArray(&memory, meshVertexArrayHandle);
                rendererDrawElementsInstanced(primitiveType, elementCount, 1);
            }

            appCtx.render();

            // process events
            glfw.processEvents();
        }

        return 0;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unhandled exception thrown." << std::endl;
        return 1;
    }
}