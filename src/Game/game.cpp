#include "game.h"

#include "../Engine/terrain_renderer.h"
#include "../Engine/terrain_assets.h"
#include "../Engine/IO/Path.hpp"

float clamp(float value, float min, float max)
{
    return (value < min ? min : (value > max ? max : value));
}

void reloadHeightmap(GameMemory *memory,
    Heightfield *heightfield,
    uint32 textureHandle,
    const char *relativePath)
{
    PlatformReadFileResult result = memory->platformReadFile(
        Terrain::Engine::IO::Path::getAbsolutePath(relativePath).c_str());
    assert(result.data);

    TextureAsset asset;
    assetsLoadTexture(&memory->engine, result.data, result.size, true, &asset);
    rendererUpdateTexture(&memory->engine, textureHandle, GL_UNSIGNED_SHORT, GL_R16, GL_RED,
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

    memory->platformFreeMemory(result.data);
}

void initializeGame(GameMemory *memory)
{
    GameState *state = &memory->state;

    rendererInitialize(&memory->engine);

    state->isOrbitCameraMode = false;
    state->isWireframeMode = false;
    state->isLightingEnabled = true;
    state->isAlbedoEnabled = true;
    state->isNormalMapEnabled = true;
    state->isAOMapEnabled = true;
    state->isDisplacementMapEnabled = true;

    state->orbitCameraDistance = 112.5f;
    state->orbitCameraPos = glm::vec3(0);
    state->orbitCameraLookAt = glm::vec3(0);
    state->orbitCameraPitch = glm::radians(15.0f);
    state->orbitCameraYaw = glm::radians(90.0f);

    state->firstPersonCameraYaw = -1.57f;
    state->firstPersonCameraPitch = 0.0f;
    state->firstPersonCameraPos = glm::vec3(0, 4, 50);
    state->firstPersonCameraLookAt = glm::vec3(0, 4, 49);

    state->heightfield = {};
    state->heightfield.columns = HEIGHTFIELD_COLUMNS;
    state->heightfield.rows = HEIGHTFIELD_ROWS;
    state->heightfield.spacing = 0.5f;
    state->heightfield.maxHeight = 25.0f;
    state->heightfield.heights = state->heightfieldHeights;
    state->heightfield.position = glm::vec2(-63.75f, -63.75f);
    *state->heightfieldHeights = {0};

    state->terrainMeshElementCount =
        (state->heightfield.rows - 1) * (state->heightfield.columns - 1) * 4;
    uint32 vertexBufferStride = 5 * sizeof(float);
    uint32 vertexBufferSize =
        state->heightfield.columns * state->heightfield.rows * vertexBufferStride;
    float *vertices = (float *)malloc(vertexBufferSize);

    uint32 elementBufferSize = sizeof(uint32) * state->terrainMeshElementCount;
    uint32 *indices = (uint32 *)malloc(elementBufferSize);

    float offsetX = (state->heightfield.columns - 1) * state->heightfield.spacing * -0.5f;
    float offsetY = (state->heightfield.rows - 1) * state->heightfield.spacing * -0.5f;
    glm::vec2 uvSize = glm::vec2(
        1.0f / (state->heightfield.columns - 1), 1.0f / (state->heightfield.rows - 1));

    float *currentVertex = vertices;
    uint32 *currentIndex = indices;
    for (uint32 y = 0; y < state->heightfield.rows; y++)
    {
        for (uint32 x = 0; x < state->heightfield.columns; x++)
        {
            *currentVertex++ = (x * state->heightfield.spacing) + offsetX;
            *currentVertex++ = 0;
            *currentVertex++ = (y * state->heightfield.spacing) + offsetY;
            *currentVertex++ = uvSize.x * x;
            *currentVertex++ = uvSize.y * y;

            if (y < state->heightfield.rows - 1 && x < state->heightfield.columns - 1)
            {
                uint32 patchIndex = (y * state->heightfield.columns) + x;
                *currentIndex++ = patchIndex;
                *currentIndex++ = patchIndex + state->heightfield.columns;
                *currentIndex++ = patchIndex + state->heightfield.columns + 1;
                *currentIndex++ = patchIndex + 1;
            }
        }
    }

    state->terrainMeshVertexBufferHandle =
        rendererCreateBuffer(&memory->engine, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(
        &memory->engine, state->terrainMeshVertexBufferHandle, vertexBufferSize, vertices);
    free(vertices);

    uint32 terrainMeshElementBufferHandle =
        rendererCreateBuffer(&memory->engine, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(
        &memory->engine, terrainMeshElementBufferHandle, elementBufferSize, indices);
    free(indices);

    state->terrainMeshVertexArrayHandle = rendererCreateVertexArray(&memory->engine);
    rendererBindVertexArray(&memory->engine, state->terrainMeshVertexArrayHandle);
    rendererBindBuffer(&memory->engine, terrainMeshElementBufferHandle);
    rendererBindBuffer(&memory->engine, state->terrainMeshVertexBufferHandle);
    rendererBindVertexAttribute(0, GL_FLOAT, false, 3, vertexBufferStride, 0, false);
    rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, vertexBufferStride, 3 * sizeof(float), false);
    rendererUnbindVertexArray();

    state->terrainMeshTessLevelBufferHandle =
        rendererCreateBuffer(&memory->engine, RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
    rendererUpdateBuffer(&memory->engine, state->terrainMeshTessLevelBufferHandle,
        state->heightfield.columns * state->heightfield.rows * sizeof(glm::vec4), 0);

    state->heightmapTextureHandle = rendererCreateTexture(&memory->engine, GL_UNSIGNED_SHORT,
        GL_R16, GL_RED, 2048, 2048, GL_MIRRORED_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    reloadHeightmap(
        memory, &state->heightfield, state->heightmapTextureHandle, "data/heightmap.tga");

    state->albedoTextureArrayHandle =
        rendererCreateTextureArray(&memory->engine, GL_UNSIGNED_BYTE, GL_RGBA, GL_RGB, 2048,
            2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    state->normalTextureArrayHandle =
        rendererCreateTextureArray(&memory->engine, GL_UNSIGNED_BYTE, GL_RGB, GL_RGB, 2048,
            2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    state->displacementTextureArrayHandle =
        rendererCreateTextureArray(&memory->engine, GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048,
            2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    state->aoTextureArrayHandle = rendererCreateTextureArray(&memory->engine, GL_UNSIGNED_BYTE,
        GL_R8, GL_RED, 2048, 2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

    state->groundAlbedoTextureVersion = 0;
    state->rockAlbedoTextureVersion = 0;
    state->snowAlbedoTextureVersion = 0;
    state->groundNormalTextureVersion = 0;
    state->rockNormalTextureVersion = 0;
    state->snowNormalTextureVersion = 0;
    state->groundDisplacementTextureVersion = 0;
    state->rockDisplacementTextureVersion = 0;
    state->snowDisplacementTextureVersion = 0;
    state->groundAoTextureVersion = 0;
    state->rockAoTextureVersion = 0;
    state->snowAoTextureVersion = 0;

    /*
        Material properties consist of:
          1. Texture Size in world units (vec2)
          2. Padding (vec2)
          3. Min Slope (float)
          4. Max Slope (float)
          5. Min Altitude (float)
          6. Max Altitude (float)
    */
    float materialProps[MATERIAL_COUNT * 8];
    materialProps[0] = 2.5f;
    materialProps[1] = 2.5f;
    materialProps[2] = 0.0f;
    materialProps[3] = 0.0f;
    materialProps[4] = 0.0f;
    materialProps[5] = 0.0f;
    materialProps[6] = 0.0f;
    materialProps[7] = 0.0f;

    materialProps[8] = 13.0f;
    materialProps[9] = 13.0f;
    materialProps[10] = 0.0f;
    materialProps[11] = 0.0f;
    materialProps[12] = 0.6f;
    materialProps[13] = 0.8f;
    materialProps[14] = 0.0f;
    materialProps[15] = 0.001f;

    materialProps[16] = 2.0f;
    materialProps[17] = 2.0f;
    materialProps[18] = 0.0f;
    materialProps[19] = 0.0f;
    materialProps[20] = 0.8f;
    materialProps[21] = 0.75f;
    materialProps[22] = 0.25f;
    materialProps[23] = 0.28f;

    state->materialPropsBufferHandle =
        rendererCreateBuffer(&memory->engine, RENDERER_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
    rendererUpdateBuffer(&memory->engine, state->materialPropsBufferHandle,
        sizeof(materialProps), materialProps);
}

void gameUpdateAndRender(GameMemory *memory, Viewport viewport, float deltaTime)
{
    if (!memory->isInitialized)
    {
        initializeGame(memory);
        memory->isInitialized = true;
    }

    Terrain::Engine::IO::InputManager *input = memory->input;
    GameState *state = &memory->state;

    bool isLightingStateUpdated = false;
    glm::vec4 lightDir = glm::vec4(-0.588f, 0.809f, 0.294f, 0.0f);

    // swap camera mode when C key is pressed
    if (input->isNewKeyPress(0, Terrain::Engine::IO::Key::C))
    {
        state->isOrbitCameraMode = !state->isOrbitCameraMode;
    }

    // toggle lighting when L key is pressed
    if (input->isNewKeyPress(0, Terrain::Engine::IO::Key::L))
    {
        state->isLightingEnabled = !state->isLightingEnabled;
        isLightingStateUpdated = true;
    }

    // toggle albedo texture when T key is pressed
    if (input->isNewKeyPress(0, Terrain::Engine::IO::Key::T))
    {
        state->isAlbedoEnabled = !state->isAlbedoEnabled;
        isLightingStateUpdated = true;
    }

    // toggle normal map texture when N key is pressed
    if (input->isNewKeyPress(0, Terrain::Engine::IO::Key::N))
    {
        state->isNormalMapEnabled = !state->isNormalMapEnabled;
        isLightingStateUpdated = true;
    }

    // toggle displacement map texture when B key is pressed
    if (input->isNewKeyPress(0, Terrain::Engine::IO::Key::B))
    {
        state->isDisplacementMapEnabled = !state->isDisplacementMapEnabled;
        isLightingStateUpdated = true;
    }

    // toggle ambient occlusion texture when O key is pressed
    if (input->isNewKeyPress(0, Terrain::Engine::IO::Key::O))
    {
        state->isAOMapEnabled = !state->isAOMapEnabled;
        isLightingStateUpdated = true;
    }

    // load a different heightmap when H is pressed
    if (input->isNewKeyPress(0, Terrain::Engine::IO::Key::H))
    {
        reloadHeightmap(
            memory, &state->heightfield, state->heightmapTextureHandle, "data/heightmap2.tga");
    }

    // toggle terrain wireframe mode when Z is pressed
    if (input->isNewKeyPress(0, Terrain::Engine::IO::Key::Z))
    {
        state->isWireframeMode = !state->isWireframeMode;
    }

    if (isLightingStateUpdated)
    {
        rendererUpdateLightingState(&memory->engine, &lightDir, state->isLightingEnabled,
            state->isAlbedoEnabled, state->isNormalMapEnabled, state->isAOMapEnabled,
            state->isDisplacementMapEnabled);
        isLightingStateUpdated = false;
    }

    if (state->isOrbitCameraMode)
    {
        bool isManipulatingOrbitCamera = false;
        const Terrain::Engine::IO::MouseInputState &mouseState = input->getMouseState(0);

        // orbit distance is modified by scrolling the mouse wheel
        state->orbitCameraDistance *= 1.0f - (glm::sign(mouseState.scrollOffsetY) * 0.05f);

        // only update the look at position if the middle mouse button is pressed
        if (input->isMouseButtonDown(0, Terrain::Engine::IO::MouseButton::Middle))
        {
            glm::vec3 lookDir =
                glm::normalize(state->orbitCameraLookAt - state->orbitCameraPos);
            glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
            glm::vec3 yDir = cross(lookDir, xDir);
            glm::vec3 pan =
                (xDir * mouseState.cursorOffsetX) + (yDir * mouseState.cursorOffsetY);
            state->orbitCameraLookAt +=
                pan * clamp(state->orbitCameraDistance, 2.5f, 300.0f) * 0.02f * deltaTime;

            isManipulatingOrbitCamera = true;
        }

        // only update yaw & pitch if the right mouse button is pressed
        if (input->isMouseButtonDown(0, Terrain::Engine::IO::MouseButton::Right))
        {
            float rotateSensitivity =
                0.05f * clamp(state->orbitCameraDistance, 14.0f, 70.0f) * deltaTime;
            state->orbitCameraYaw +=
                glm::radians(mouseState.cursorOffsetX * rotateSensitivity);
            state->orbitCameraPitch +=
                glm::radians(mouseState.cursorOffsetY * rotateSensitivity);

            isManipulatingOrbitCamera = true;
        }

        // calculate camera position
        glm::vec3 newLookDir =
            glm::vec3(cos(state->orbitCameraYaw) * cos(state->orbitCameraPitch),
                sin(state->orbitCameraPitch),
                sin(state->orbitCameraYaw) * cos(state->orbitCameraPitch));
        state->orbitCameraPos =
            state->orbitCameraLookAt + (newLookDir * state->orbitCameraDistance);

        // capture mouse if orbit camera is being manipulated
        if (isManipulatingOrbitCamera)
        {
            input->captureMouse(false);
        }
    }
    else
    {
        const float lookSensitivity = 0.07f * deltaTime;
        const float moveSpeed = 4.0 * deltaTime;
        const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        const Terrain::Engine::IO::MouseInputState &mouseState = input->getMouseState(0);

        // rotate camera by moving mouse cursor
        state->firstPersonCameraYaw += mouseState.cursorOffsetX * lookSensitivity;
        state->firstPersonCameraPitch = clamp(state->firstPersonCameraPitch
                - ((float)mouseState.cursorOffsetY * lookSensitivity),
            -1.55f, 1.55f);
        glm::vec3 lookDir =
            glm::vec3(cos(state->firstPersonCameraYaw) * cos(state->firstPersonCameraPitch),
                sin(state->firstPersonCameraPitch),
                sin(state->firstPersonCameraYaw) * cos(state->firstPersonCameraPitch));

        // move camera on XZ axis using WASD keys
        glm::vec3 moveDir = glm::vec3(
            cos(state->firstPersonCameraYaw), 0.0f, sin(state->firstPersonCameraYaw));
        if (input->isKeyDown(0, Terrain::Engine::IO::Key::A))
        {
            state->firstPersonCameraPos -= glm::normalize(glm::cross(moveDir, up)) * moveSpeed;
        }
        if (input->isKeyDown(0, Terrain::Engine::IO::Key::D))
        {
            state->firstPersonCameraPos += glm::normalize(glm::cross(moveDir, up)) * moveSpeed;
        }
        if (input->isKeyDown(0, Terrain::Engine::IO::Key::W))
        {
            state->firstPersonCameraPos += moveDir * moveSpeed;
        }
        if (input->isKeyDown(0, Terrain::Engine::IO::Key::S))
        {
            state->firstPersonCameraPos -= moveDir * moveSpeed;
        }

        // smoothly lerp Y to terrain height
        float targetHeight = heightfieldGetHeight(&state->heightfield,
                                 state->firstPersonCameraPos.x, state->firstPersonCameraPos.z)
            + 1.75f;
        state->firstPersonCameraPos.y =
            (state->firstPersonCameraPos.y * 0.95f) + (targetHeight * 0.05f);

        state->firstPersonCameraLookAt = state->firstPersonCameraPos + lookDir;

        // capture mouse if first person camera is active
        input->captureMouse(false);
    }

    // render world
    constexpr float fov = glm::pi<float>() / 4.0f;
    const float nearPlane = 0.1f;
    const float farPlane = 10000.0f;
    const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    const float aspectRatio = (float)viewport.width / (float)viewport.height;

    glm::vec3 *cameraPos =
        state->isOrbitCameraMode ? &state->orbitCameraPos : &state->firstPersonCameraPos;
    glm::vec3 *cameraLookAt =
        state->isOrbitCameraMode ? &state->orbitCameraLookAt : &state->firstPersonCameraLookAt;
    glm::mat4 cameraTransform = glm::perspective(fov, aspectRatio, nearPlane, farPlane)
        * glm::lookAt(*cameraPos, *cameraLookAt, up);

    rendererUpdateCameraState(&memory->engine, &cameraTransform);
    rendererSetViewportSize(viewport.width, viewport.height);
    rendererClearBackBuffer(0.392f, 0.584f, 0.929f, 1);

    TextureAsset *asset;
    asset = assetsGetTexture(&memory->engine, ASSET_TEXTURE_GROUND_ALBEDO);
    if (asset && asset->version > state->groundAlbedoTextureVersion)
    {
        rendererUpdateTextureArray(&memory->engine, state->albedoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, 0, asset->data);
        state->groundAlbedoTextureVersion = asset->version;
    }
    asset = assetsGetTexture(&memory->engine, ASSET_TEXTURE_GROUND_NORMAL);
    if (asset && asset->version > state->groundNormalTextureVersion)
    {
        rendererUpdateTextureArray(&memory->engine, state->normalTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, 0, asset->data);
        state->groundNormalTextureVersion = asset->version;
    }
    asset = assetsGetTexture(&memory->engine, ASSET_TEXTURE_GROUND_DISPLACEMENT);
    if (asset && asset->version > state->groundDisplacementTextureVersion)
    {
        rendererUpdateTextureArray(&memory->engine, state->displacementTextureArrayHandle,
            GL_UNSIGNED_SHORT, GL_RED, asset->width, asset->height, 0, asset->data);
        state->groundDisplacementTextureVersion = asset->version;
    }
    asset = assetsGetTexture(&memory->engine, ASSET_TEXTURE_GROUND_AO);
    if (asset && asset->version > state->groundAoTextureVersion)
    {
        rendererUpdateTextureArray(&memory->engine, state->aoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RED, asset->width, asset->height, 0, asset->data);
        state->groundAoTextureVersion = asset->version;
    }
    asset = assetsGetTexture(&memory->engine, ASSET_TEXTURE_ROCK_ALBEDO);
    if (asset && asset->version > state->rockAlbedoTextureVersion)
    {
        rendererUpdateTextureArray(&memory->engine, state->albedoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, 1, asset->data);
        state->rockAlbedoTextureVersion = asset->version;
    }
    asset = assetsGetTexture(&memory->engine, ASSET_TEXTURE_ROCK_NORMAL);
    if (asset && asset->version > state->rockNormalTextureVersion)
    {
        rendererUpdateTextureArray(&memory->engine, state->normalTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, 1, asset->data);
        state->rockNormalTextureVersion = asset->version;
    }
    asset = assetsGetTexture(&memory->engine, ASSET_TEXTURE_ROCK_DISPLACEMENT);
    if (asset && asset->version > state->rockDisplacementTextureVersion)
    {
        rendererUpdateTextureArray(&memory->engine, state->displacementTextureArrayHandle,
            GL_UNSIGNED_SHORT, GL_RED, asset->width, asset->height, 1, asset->data);
        state->rockDisplacementTextureVersion = asset->version;
    }
    asset = assetsGetTexture(&memory->engine, ASSET_TEXTURE_ROCK_AO);
    if (asset && asset->version > state->rockAoTextureVersion)
    {
        rendererUpdateTextureArray(&memory->engine, state->aoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RED, asset->width, asset->height, 1, asset->data);
        state->rockAoTextureVersion = asset->version;
    }
    asset = assetsGetTexture(&memory->engine, ASSET_TEXTURE_SNOW_ALBEDO);
    if (asset && asset->version > state->snowAlbedoTextureVersion)
    {
        rendererUpdateTextureArray(&memory->engine, state->albedoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, 2, asset->data);
        state->snowAlbedoTextureVersion = asset->version;
    }
    asset = assetsGetTexture(&memory->engine, ASSET_TEXTURE_SNOW_NORMAL);
    if (asset && asset->version > state->snowNormalTextureVersion)
    {
        rendererUpdateTextureArray(&memory->engine, state->normalTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, 2, asset->data);
        state->snowNormalTextureVersion = asset->version;
    }
    asset = assetsGetTexture(&memory->engine, ASSET_TEXTURE_SNOW_DISPLACEMENT);
    if (asset && asset->version > state->snowDisplacementTextureVersion)
    {
        rendererUpdateTextureArray(&memory->engine, state->displacementTextureArrayHandle,
            GL_UNSIGNED_SHORT, GL_RED, asset->width, asset->height, 2, asset->data);
        state->snowDisplacementTextureVersion = asset->version;
    }
    asset = assetsGetTexture(&memory->engine, ASSET_TEXTURE_SNOW_AO);
    if (asset && asset->version > state->snowAoTextureVersion)
    {
        rendererUpdateTextureArray(&memory->engine, state->aoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RED, asset->width, asset->height, 2, asset->data);
        state->snowAoTextureVersion = asset->version;
    }

    uint32 terrainShaderProgramAssetId = state->isWireframeMode
        ? ASSET_SHADER_PROGRAM_TERRAIN_WIREFRAME
        : ASSET_SHADER_PROGRAM_TERRAIN_TEXTURED;
    uint32 terrainPolygonMode = state->isWireframeMode ? GL_LINE : GL_FILL;

    ShaderProgramAsset *calcTessLevelShaderProgram =
        assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL);
    ShaderProgramAsset *terrainShaderProgram =
        assetsGetShaderProgram(&memory->engine, terrainShaderProgramAssetId);
    if (calcTessLevelShaderProgram && terrainShaderProgram)
    {
        uint32 meshEdgeCount = (2 * (state->heightfield.rows * state->heightfield.columns))
            - state->heightfield.rows - state->heightfield.columns;

        rendererSetShaderProgramUniformFloat(
            &memory->engine, calcTessLevelShaderProgram->handle, "targetTriangleSize", 0.015f);
        rendererSetShaderProgramUniformInteger(&memory->engine,
            calcTessLevelShaderProgram->handle, "horizontalEdgeCount",
            state->heightfield.rows * (state->heightfield.columns - 1));
        rendererSetShaderProgramUniformInteger(&memory->engine,
            calcTessLevelShaderProgram->handle, "columnCount", state->heightfield.columns);
        rendererSetShaderProgramUniformFloat(&memory->engine,
            calcTessLevelShaderProgram->handle, "terrainHeight", state->heightfield.maxHeight);
        rendererBindTexture(&memory->engine, state->heightmapTextureHandle, 0);
        rendererBindTexture(&memory->engine, state->heightmapTextureHandle, 5);
        rendererBindShaderStorageBuffer(
            &memory->engine, state->terrainMeshTessLevelBufferHandle, 0);
        rendererBindShaderStorageBuffer(
            &memory->engine, state->terrainMeshVertexBufferHandle, 1);
        rendererUseShaderProgram(&memory->engine, calcTessLevelShaderProgram->handle);
        rendererDispatchCompute(meshEdgeCount, 1, 1);
        rendererShaderStorageMemoryBarrier();

        rendererUseShaderProgram(&memory->engine, terrainShaderProgram->handle);
        rendererSetPolygonMode(terrainPolygonMode);
        rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        rendererBindShaderStorageBuffer(&memory->engine, state->materialPropsBufferHandle, 1);
        rendererSetShaderProgramUniformVector2(&memory->engine, terrainShaderProgram->handle,
            "brushHighlightPos", glm::vec2(0.0f, 0.0f));
        rendererSetShaderProgramUniformFloat(
            &memory->engine, terrainShaderProgram->handle, "brushHighlightStrength", 0.0f);
        rendererSetShaderProgramUniformFloat(
            &memory->engine, terrainShaderProgram->handle, "brushHighlightRadius", 0.0f);
        rendererSetShaderProgramUniformFloat(
            &memory->engine, terrainShaderProgram->handle, "brushHighlightFalloff", 0.0f);
        rendererSetShaderProgramUniformVector3(
            &memory->engine, terrainShaderProgram->handle, "color", glm::vec3(0, 1, 0));
        rendererSetShaderProgramUniformVector3(&memory->engine, terrainShaderProgram->handle,
            "terrainDimensions",
            glm::vec3(state->heightfield.spacing * state->heightfield.columns,
                state->heightfield.maxHeight,
                state->heightfield.spacing * state->heightfield.rows));
        rendererSetShaderProgramUniformInteger(
            &memory->engine, terrainShaderProgram->handle, "materialCount", MATERIAL_COUNT);
        rendererBindTexture(&memory->engine, state->heightmapTextureHandle, 0);
        rendererBindTextureArray(&memory->engine, state->albedoTextureArrayHandle, 1);
        rendererBindTextureArray(&memory->engine, state->normalTextureArrayHandle, 2);
        rendererBindTextureArray(&memory->engine, state->displacementTextureArrayHandle, 3);
        rendererBindTextureArray(&memory->engine, state->aoTextureArrayHandle, 4);
        rendererBindTexture(&memory->engine, state->heightmapTextureHandle, 5);
        rendererBindVertexArray(&memory->engine, state->terrainMeshVertexArrayHandle);
        rendererDrawElements(GL_PATCHES, state->terrainMeshElementCount);
    }
}

void gameShutdown(GameMemory *memory)
{
    rendererDestroyResources(&memory->engine);
}