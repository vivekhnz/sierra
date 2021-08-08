#include "game.h"

#include "../Engine/engine.h"

#define MAX_PATH 260
#define arrayCount(array) (sizeof(array) / sizeof(array[0]))

float clamp(float value, float min, float max)
{
    return (value < min ? min : (value > max ? max : value));
}

bool initializeGame(GameMemory *memory)
{
    GameState *state = &memory->state;
    GameAssets *gameAssets = &state->gameAssets;
    EngineApi *engine = memory->engine;
    MemoryArena *arena = &memory->arena;

    state->renderCtx = engine->rendererInitialize(arena);

    state->assetsArena = pushSubArena(arena, 200 * 1024 * 1024);
    state->engineAssets = engine->assetsInitialize(&state->assetsArena, state->renderCtx);
    Assets *assets = state->engineAssets;

    gameAssets->terrainShaderTextured =
        engine->assetsRegisterShader(assets, "terrain_textured.fs.glsl", SHADER_TYPE_TERRAIN);
    gameAssets->terrainShaderWireframe =
        engine->assetsRegisterShader(assets, "terrain_wireframe.fs.glsl", SHADER_TYPE_TERRAIN);

    gameAssets->textureGroundAlbedo = engine->assetsRegisterTexture(assets, "ground_albedo.bmp", false);
    gameAssets->textureGroundNormal = engine->assetsRegisterTexture(assets, "ground_normal.bmp", false);
    gameAssets->textureGroundDisplacement = engine->assetsRegisterTexture(assets, "ground_displacement.tga", true);
    gameAssets->textureGroundAo = engine->assetsRegisterTexture(assets, "ground_ao.tga", false);
    gameAssets->textureRockAlbedo = engine->assetsRegisterTexture(assets, "rock_albedo.jpg", false);
    gameAssets->textureRockNormal = engine->assetsRegisterTexture(assets, "rock_normal.jpg", false);
    gameAssets->textureRockDisplacement = engine->assetsRegisterTexture(assets, "rock_displacement.tga", true);
    gameAssets->textureRockAo = engine->assetsRegisterTexture(assets, "rock_ao.tga", false);
    gameAssets->textureSnowAlbedo = engine->assetsRegisterTexture(assets, "snow_albedo.jpg", false);
    gameAssets->textureSnowNormal = engine->assetsRegisterTexture(assets, "snow_normal.jpg", false);
    gameAssets->textureSnowDisplacement = engine->assetsRegisterTexture(assets, "snow_displacement.tga", true);
    gameAssets->textureSnowAo = engine->assetsRegisterTexture(assets, "snow_ao.tga", false);

    gameAssets->textureVirtualHeightmap = engine->assetsRegisterTexture(assets, 0, true);

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

    float tileLengthInWorldUnits = 128.0f;
    state->heightfield = {};
    state->heightfield.columns = HEIGHTFIELD_COLUMNS;
    state->heightfield.rows = HEIGHTFIELD_ROWS;
    state->heightfield.spacing = tileLengthInWorldUnits / (HEIGHTFIELD_COLUMNS - 1);
    state->heightfield.maxHeight = 25.0f;
    state->heightfield.heights = state->heightfieldHeights;
    state->heightfield.center = glm::vec2(0, 0);
    *state->heightfieldHeights = {0};

    TemporaryMemory terrainMeshMemory = beginTemporaryMemory(arena);

    uint32 terrainMeshVertsPerEdge = state->heightfield.columns;
    uint32 terrainMeshVertexCount = terrainMeshVertsPerEdge * terrainMeshVertsPerEdge;
    uint32 terrainMeshVertexSize = 5;
    glm::vec3 terrainBoundsMin = glm::vec3(tileLengthInWorldUnits * -0.5f, 0, tileLengthInWorldUnits * -0.5f);
    float terrainMeshSpacing = tileLengthInWorldUnits / (terrainMeshVertsPerEdge - 1);
    uint32 terrainMeshElementCount = (terrainMeshVertsPerEdge - 1) * (terrainMeshVertsPerEdge - 1) * 4;

    float *terrainVertices = pushArray(arena, float, (terrainMeshVertexCount * terrainMeshVertexSize));
    uint32 *terrainIndices = pushArray(arena, uint32, terrainMeshElementCount);

    float *currentVertex = terrainVertices;
    uint32 *currentIndex = terrainIndices;
    for (uint32 y = 0; y < terrainMeshVertsPerEdge; y++)
    {
        for (uint32 x = 0; x < terrainMeshVertsPerEdge; x++)
        {
            *currentVertex++ = terrainBoundsMin.x + (x * terrainMeshSpacing);
            *currentVertex++ = terrainBoundsMin.y;
            *currentVertex++ = terrainBoundsMin.z + (y * terrainMeshSpacing);
            *currentVertex++ = x / (float)(terrainMeshVertsPerEdge - 1);
            *currentVertex++ = y / (float)(terrainMeshVertsPerEdge - 1);

            if (y < terrainMeshVertsPerEdge - 1 && x < terrainMeshVertsPerEdge - 1)
            {
                uint32 patchIndex = (y * terrainMeshVertsPerEdge) + x;
                *currentIndex++ = patchIndex;
                *currentIndex++ = patchIndex + terrainMeshVertsPerEdge;
                *currentIndex++ = patchIndex + terrainMeshVertsPerEdge + 1;
                *currentIndex++ = patchIndex + 1;
            }
        }
    }

    state->terrainMeshElementCount = terrainMeshElementCount;
    state->terrainMeshVertexBuffer = engine->rendererCreateBuffer(RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(&state->terrainMeshVertexBuffer,
        terrainMeshVertexCount * terrainMeshVertexSize * sizeof(float), terrainVertices);
    state->terrainMeshElementBuffer = engine->rendererCreateBuffer(RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(
        &state->terrainMeshElementBuffer, terrainMeshElementCount * sizeof(uint32), terrainIndices);
    state->terrainMeshTessLevelBuffer =
        engine->rendererCreateBuffer(RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
    engine->rendererUpdateBuffer(
        &state->terrainMeshTessLevelBuffer, terrainMeshVertexCount * sizeof(glm::vec4), 0);

    endTemporaryMemory(&terrainMeshMemory);

    state->heightmapTexture = engine->rendererCreateTexture(2048, 2048, TEXTURE_FORMAT_R16);
    memory->platformQueueAssetLoad(gameAssets->textureVirtualHeightmap, "heightmap.tga");

    state->albedoTextureArrayId = engine->rendererCreateTextureArray(
        GL_UNSIGNED_BYTE, GL_RGBA, GL_RGB, 2048, 2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    state->normalTextureArrayId = engine->rendererCreateTextureArray(
        GL_UNSIGNED_BYTE, GL_RGB, GL_RGB, 2048, 2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    state->displacementTextureArrayId = engine->rendererCreateTextureArray(
        GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048, 2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    state->aoTextureArrayId = engine->rendererCreateTextureArray(
        GL_UNSIGNED_BYTE, GL_R8, GL_RED, 2048, 2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

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
    materialProps[12] = 0.2f;
    materialProps[13] = 0.4f;
    materialProps[14] = 0.0f;
    materialProps[15] = 0.001f;

    materialProps[16] = 2.0f;
    materialProps[17] = 2.0f;
    materialProps[18] = 0.0f;
    materialProps[19] = 0.0f;
    materialProps[20] = 0.4f;
    materialProps[21] = 0.2f;
    materialProps[22] = 0.25f;
    materialProps[23] = 0.28f;

    state->materialPropsBuffer = engine->rendererCreateBuffer(RENDERER_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
    engine->rendererUpdateBuffer(&state->materialPropsBuffer, sizeof(materialProps), materialProps);

    return 1;
}

bool isButtonDown(GameInput *input, GameInputButtons button)
{
    return input->pressedButtons & button;
}

bool isNewButtonPress(GameInput *input, GameInputButtons button)
{
    return (input->pressedButtons & button) && !(input->prevPressedButtons & button);
}

API_EXPORT GAME_UPDATE_AND_RENDER(gameUpdateAndRender)
{
    if (!memory->isInitialized)
    {
        if (!initializeGame(memory))
        {
            memory->platformExitGame();
            return;
        }
        memory->isInitialized = true;
    }

    EngineApi *engine = memory->engine;
    GameState *state = &memory->state;
    GameAssets *gameAssets = &state->gameAssets;
    RenderContext *rctx = state->renderCtx;

    glm::vec4 lightDir = glm::vec4(-0.588f, 0.809f, 0.294f, 0.0f);

    if (isButtonDown(input, GAME_INPUT_KEY_ESCAPE))
    {
        memory->platformExitGame();
    }

    LoadedAsset *heightmapAsset = engine->assetsGetTexture(gameAssets->textureVirtualHeightmap);
    if (heightmapAsset->texture && heightmapAsset->version != state->heightmapTextureVersion)
    {
        memory->engine->rendererUpdateTexture(state->heightmapTexture, heightmapAsset->texture->width,
            heightmapAsset->texture->height, heightmapAsset->texture->data);

        uint16 heightmapWidth = 2048;
        uint16 heightmapHeight = 2048;
        uint16 patchTexelWidth = heightmapWidth / state->heightfield.columns;
        uint16 patchTexelHeight = heightmapHeight / state->heightfield.rows;

        uint16 *src = (uint16 *)heightmapAsset->texture->data;
        float *dst = (float *)state->heightfield.heights;
        float heightScalar = state->heightfield.maxHeight / (float)UINT16_MAX;
        for (uint32 y = 0; y < state->heightfield.rows; y++)
        {
            for (uint32 x = 0; x < state->heightfield.columns; x++)
            {
                *dst++ = *src * heightScalar;
                src += patchTexelWidth;
            }
            src += (patchTexelHeight - 1) * heightmapWidth;
        }

        state->heightmapTextureVersion = heightmapAsset->version;
    }

    // swap camera mode when C key is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_C))
    {
        state->isOrbitCameraMode = !state->isOrbitCameraMode;
    }

    // toggle lighting when L key is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_L))
    {
        state->isLightingEnabled = !state->isLightingEnabled;
    }

    // toggle albedo texture when T key is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_T))
    {
        state->isAlbedoEnabled = !state->isAlbedoEnabled;
    }

    // toggle normal map texture when N key is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_N))
    {
        state->isNormalMapEnabled = !state->isNormalMapEnabled;
    }

    // toggle displacement map texture when B key is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_B))
    {
        state->isDisplacementMapEnabled = !state->isDisplacementMapEnabled;
    }

    // toggle ambient occlusion texture when O key is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_O))
    {
        state->isAOMapEnabled = !state->isAOMapEnabled;
    }

    // load a different heightmap when H is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_H))
    {
        memory->platformQueueAssetLoad(gameAssets->textureVirtualHeightmap, "heightmap2.tga");
    }

    // toggle terrain wireframe mode when Z is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_Z))
    {
        state->isWireframeMode = !state->isWireframeMode;
    }

    if (state->isOrbitCameraMode)
    {
        bool isManipulatingOrbitCamera = false;

        // orbit distance is modified by scrolling the mouse wheel
        state->orbitCameraDistance *= 1.0f - (glm::sign(input->mouseScrollOffset) * 0.05f);

        // only update the look at position if the middle mouse button is pressed
        if (isButtonDown(input, GAME_INPUT_MOUSE_MIDDLE))
        {
            glm::vec3 lookDir = glm::normalize(state->orbitCameraLookAt - state->orbitCameraPos);
            glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
            glm::vec3 yDir = cross(lookDir, xDir);
            glm::vec3 pan = (xDir * input->mouseCursorOffset.x) + (yDir * input->mouseCursorOffset.y);
            state->orbitCameraLookAt += pan * clamp(state->orbitCameraDistance, 2.5f, 300.0f) * 0.02f * deltaTime;

            isManipulatingOrbitCamera = true;
        }

        // only update yaw & pitch if the right mouse button is pressed
        if (isButtonDown(input, GAME_INPUT_MOUSE_RIGHT))
        {
            float rotateSensitivity = 0.05f * clamp(state->orbitCameraDistance, 14.0f, 70.0f) * deltaTime;
            state->orbitCameraYaw += glm::radians(input->mouseCursorOffset.x * rotateSensitivity);
            state->orbitCameraPitch += glm::radians(input->mouseCursorOffset.y * rotateSensitivity);

            isManipulatingOrbitCamera = true;
        }

        // calculate camera position
        glm::vec3 newLookDir = glm::vec3(cos(state->orbitCameraYaw) * cos(state->orbitCameraPitch),
            sin(state->orbitCameraPitch), sin(state->orbitCameraYaw) * cos(state->orbitCameraPitch));
        state->orbitCameraPos = state->orbitCameraLookAt + (newLookDir * state->orbitCameraDistance);

        // capture mouse if orbit camera is being manipulated
        if (isManipulatingOrbitCamera)
        {
            memory->platformCaptureMouse();
        }
    }
    else
    {
        const float lookSensitivity = 0.07f * deltaTime;
        const float moveSpeed = 4.0 * deltaTime;
        const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        // rotate camera by moving mouse cursor
        state->firstPersonCameraYaw += input->mouseCursorOffset.x * lookSensitivity;
        state->firstPersonCameraPitch = clamp(
            state->firstPersonCameraPitch - ((float)input->mouseCursorOffset.y * lookSensitivity), -1.55f, 1.55f);
        glm::vec3 lookDir = glm::vec3(cos(state->firstPersonCameraYaw) * cos(state->firstPersonCameraPitch),
            sin(state->firstPersonCameraPitch),
            sin(state->firstPersonCameraYaw) * cos(state->firstPersonCameraPitch));

        // move camera on XZ axis using WASD keys
        glm::vec3 moveDir = glm::vec3(cos(state->firstPersonCameraYaw), 0.0f, sin(state->firstPersonCameraYaw));
        if (isButtonDown(input, GAME_INPUT_KEY_A))
        {
            state->firstPersonCameraPos -= glm::normalize(glm::cross(moveDir, up)) * moveSpeed;
        }
        if (isButtonDown(input, GAME_INPUT_KEY_D))
        {
            state->firstPersonCameraPos += glm::normalize(glm::cross(moveDir, up)) * moveSpeed;
        }
        if (isButtonDown(input, GAME_INPUT_KEY_W))
        {
            state->firstPersonCameraPos += moveDir * moveSpeed;
        }
        if (isButtonDown(input, GAME_INPUT_KEY_S))
        {
            state->firstPersonCameraPos -= moveDir * moveSpeed;
        }

        // smoothly lerp Y to terrain height
        float targetHeight = engine->heightfieldGetHeight(
                                 &state->heightfield, state->firstPersonCameraPos.x, state->firstPersonCameraPos.z)
            + 1.75f;
        // state->firstPersonCameraPos.y = (state->firstPersonCameraPos.y * 0.95f) + (targetHeight * 0.05f);
        state->firstPersonCameraPos.y = targetHeight;

        state->firstPersonCameraLookAt = state->firstPersonCameraPos + lookDir;

        // capture mouse if first person camera is active
        memory->platformCaptureMouse();
    }

    // render world
    float fov = glm::pi<float>() / 4.0f;
    glm::vec3 *cameraPos = state->isOrbitCameraMode ? &state->orbitCameraPos : &state->firstPersonCameraPos;
    glm::vec3 *cameraLookAt =
        state->isOrbitCameraMode ? &state->orbitCameraLookAt : &state->firstPersonCameraLookAt;

    LoadedAsset *asset;
    asset = engine->assetsGetTexture(gameAssets->textureGroundAlbedo);
    if (asset->texture && asset->version > state->groundAlbedoTextureVersion)
    {
        engine->rendererUpdateTextureArray(state->albedoTextureArrayId, GL_UNSIGNED_BYTE, GL_RGB,
            asset->texture->width, asset->texture->height, 0, asset->texture->data);
        state->groundAlbedoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(gameAssets->textureGroundNormal);
    if (asset->texture && asset->version > state->groundNormalTextureVersion)
    {
        engine->rendererUpdateTextureArray(state->normalTextureArrayId, GL_UNSIGNED_BYTE, GL_RGB,
            asset->texture->width, asset->texture->height, 0, asset->texture->data);
        state->groundNormalTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(gameAssets->textureGroundDisplacement);
    if (asset->texture && asset->version > state->groundDisplacementTextureVersion)
    {
        engine->rendererUpdateTextureArray(state->displacementTextureArrayId, GL_UNSIGNED_SHORT, GL_RED,
            asset->texture->width, asset->texture->height, 0, asset->texture->data);
        state->groundDisplacementTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(gameAssets->textureGroundAo);
    if (asset->texture && asset->version > state->groundAoTextureVersion)
    {
        engine->rendererUpdateTextureArray(state->aoTextureArrayId, GL_UNSIGNED_BYTE, GL_RED,
            asset->texture->width, asset->texture->height, 0, asset->texture->data);
        state->groundAoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(gameAssets->textureRockAlbedo);
    if (asset->texture && asset->version > state->rockAlbedoTextureVersion)
    {
        engine->rendererUpdateTextureArray(state->albedoTextureArrayId, GL_UNSIGNED_BYTE, GL_RGB,
            asset->texture->width, asset->texture->height, 1, asset->texture->data);
        state->rockAlbedoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(gameAssets->textureRockNormal);
    if (asset->texture && asset->version > state->rockNormalTextureVersion)
    {
        engine->rendererUpdateTextureArray(state->normalTextureArrayId, GL_UNSIGNED_BYTE, GL_RGB,
            asset->texture->width, asset->texture->height, 1, asset->texture->data);
        state->rockNormalTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(gameAssets->textureRockDisplacement);
    if (asset->texture && asset->version > state->rockDisplacementTextureVersion)
    {
        engine->rendererUpdateTextureArray(state->displacementTextureArrayId, GL_UNSIGNED_SHORT, GL_RED,
            asset->texture->width, asset->texture->height, 1, asset->texture->data);
        state->rockDisplacementTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(gameAssets->textureRockAo);
    if (asset->texture && asset->version > state->rockAoTextureVersion)
    {
        engine->rendererUpdateTextureArray(state->aoTextureArrayId, GL_UNSIGNED_BYTE, GL_RED,
            asset->texture->width, asset->texture->height, 1, asset->texture->data);
        state->rockAoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(gameAssets->textureSnowAlbedo);
    if (asset->texture && asset->version > state->snowAlbedoTextureVersion)
    {
        engine->rendererUpdateTextureArray(state->albedoTextureArrayId, GL_UNSIGNED_BYTE, GL_RGB,
            asset->texture->width, asset->texture->height, 2, asset->texture->data);
        state->snowAlbedoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(gameAssets->textureSnowNormal);
    if (asset->texture && asset->version > state->snowNormalTextureVersion)
    {
        engine->rendererUpdateTextureArray(state->normalTextureArrayId, GL_UNSIGNED_BYTE, GL_RGB,
            asset->texture->width, asset->texture->height, 2, asset->texture->data);
        state->snowNormalTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(gameAssets->textureSnowDisplacement);
    if (asset->texture && asset->version > state->snowDisplacementTextureVersion)
    {
        engine->rendererUpdateTextureArray(state->displacementTextureArrayId, GL_UNSIGNED_SHORT, GL_RED,
            asset->texture->width, asset->texture->height, 2, asset->texture->data);
        state->snowDisplacementTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(gameAssets->textureSnowAo);
    if (asset->texture && asset->version > state->snowAoTextureVersion)
    {
        engine->rendererUpdateTextureArray(state->aoTextureArrayId, GL_UNSIGNED_BYTE, GL_RED,
            asset->texture->width, asset->texture->height, 2, asset->texture->data);
        state->snowAoTextureVersion = asset->version;
    }

    AssetHandle terrainShader =
        state->isWireframeMode ? gameAssets->terrainShaderWireframe : gameAssets->terrainShaderTextured;

    TemporaryMemory renderQueueMemory = beginTemporaryMemory(&memory->arena);
    glm::vec2 heightmapSize = glm::vec2(2048, 2048);

    RenderQueue *rq = engine->rendererCreateQueue(state->renderCtx, &memory->arena);
    engine->rendererSetCameraPersp(rq, *cameraPos, *cameraLookAt, fov);
    engine->rendererSetLighting(rq, &lightDir, state->isLightingEnabled, state->isAlbedoEnabled,
        state->isNormalMapEnabled, state->isAOMapEnabled, state->isDisplacementMapEnabled);
    engine->rendererClear(rq, 0.392f, 0.584f, 0.929f, 1);
    engine->rendererPushTerrain(rq, &state->heightfield, heightmapSize, terrainShader, state->heightmapTexture,
        state->heightmapTexture, {0}, {0}, {0}, {0}, {0}, {0}, state->terrainMeshVertexBuffer.id,
        state->terrainMeshElementBuffer.id, state->terrainMeshTessLevelBuffer.id, state->terrainMeshElementCount,
        MATERIAL_COUNT, state->albedoTextureArrayId, state->normalTextureArrayId,
        state->displacementTextureArrayId, state->aoTextureArrayId, state->materialPropsBuffer.id,
        state->isWireframeMode, 0, glm::vec2(0), 0, 0);
    engine->rendererDrawToScreen(rq, viewport.width, viewport.height);

    endTemporaryMemory(&renderQueueMemory);
}