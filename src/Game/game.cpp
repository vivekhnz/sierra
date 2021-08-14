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

    gameAssets->textureGroundAlbedo =
        engine->assetsRegisterTexture(assets, "ground_albedo.bmp", TEXTURE_FORMAT_RGB8);
    gameAssets->textureGroundNormal =
        engine->assetsRegisterTexture(assets, "ground_normal.bmp", TEXTURE_FORMAT_RGB8);
    gameAssets->textureGroundDisplacement =
        engine->assetsRegisterTexture(assets, "ground_displacement.tga", TEXTURE_FORMAT_R16);
    gameAssets->textureGroundAo = engine->assetsRegisterTexture(assets, "ground_ao.tga", TEXTURE_FORMAT_R8);
    gameAssets->textureRockAlbedo = engine->assetsRegisterTexture(assets, "rock_albedo.jpg", TEXTURE_FORMAT_RGB8);
    gameAssets->textureRockNormal = engine->assetsRegisterTexture(assets, "rock_normal.jpg", TEXTURE_FORMAT_RGB8);
    gameAssets->textureRockDisplacement =
        engine->assetsRegisterTexture(assets, "rock_displacement.tga", TEXTURE_FORMAT_R16);
    gameAssets->textureRockAo = engine->assetsRegisterTexture(assets, "rock_ao.tga", TEXTURE_FORMAT_R8);
    gameAssets->textureSnowAlbedo = engine->assetsRegisterTexture(assets, "snow_albedo.jpg", TEXTURE_FORMAT_RGB8);
    gameAssets->textureSnowNormal = engine->assetsRegisterTexture(assets, "snow_normal.jpg", TEXTURE_FORMAT_RGB8);
    gameAssets->textureSnowDisplacement =
        engine->assetsRegisterTexture(assets, "snow_displacement.tga", TEXTURE_FORMAT_R16);
    gameAssets->textureSnowAo = engine->assetsRegisterTexture(assets, "snow_ao.tga", TEXTURE_FORMAT_R8);

    gameAssets->textureVirtualHeightmap = engine->assetsRegisterTexture(assets, 0, TEXTURE_FORMAT_R16);

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
    state->heightfield.heightSamplesPerEdge = HEIGHTFIELD_SAMPLES_PER_EDGE;
    state->heightfield.spaceBetweenHeightSamples = tileLengthInWorldUnits / (HEIGHTFIELD_SAMPLES_PER_EDGE - 1);
    state->heightfield.maxHeight = 25.0f;
    state->heightfield.heights = state->heightfieldHeights;
    state->heightfield.center = glm::vec2(0, 0);
    *state->heightfieldHeights = {0};

    state->heightmapTexture = engine->rendererCreateTexture(2048, 2048, TEXTURE_FORMAT_R16);
    memory->platformQueueAssetLoad(gameAssets->textureVirtualHeightmap, "heightmap.tga");

    {
        TerrainMaterialTextures *textures = &state->materialTextures[0];
        textures->albedo = gameAssets->textureGroundAlbedo;
        textures->normal = gameAssets->textureGroundNormal;
        textures->displacement = gameAssets->textureGroundDisplacement;
        textures->ao = gameAssets->textureGroundAo;

        RenderTerrainMaterial *mat = &state->materials[0];
        mat->textureSizeInWorldUnits = glm::vec2(2.5f, 2.5f);
        mat->slopeStart = 0;
        mat->slopeEnd = 0;
        mat->altitudeStart = 0;
        mat->altitudeEnd = 0;
    }
    {
        TerrainMaterialTextures *textures = &state->materialTextures[1];
        textures->albedo = gameAssets->textureRockAlbedo;
        textures->normal = gameAssets->textureRockNormal;
        textures->displacement = gameAssets->textureRockDisplacement;
        textures->ao = gameAssets->textureRockAo;

        RenderTerrainMaterial *mat = &state->materials[1];
        mat->textureSizeInWorldUnits = glm::vec2(13, 13);
        mat->slopeStart = 0.2f;
        mat->slopeEnd = 0.4f;
        mat->altitudeStart = 0;
        mat->altitudeEnd = 0.001f;
    }
    {
        TerrainMaterialTextures *textures = &state->materialTextures[2];
        textures->albedo = gameAssets->textureSnowAlbedo;
        textures->normal = gameAssets->textureSnowNormal;
        textures->displacement = gameAssets->textureSnowDisplacement;
        textures->ao = gameAssets->textureSnowAo;

        RenderTerrainMaterial *mat = &state->materials[2];
        mat->textureSizeInWorldUnits = glm::vec2(2, 2);
        mat->slopeStart = 0.4f;
        mat->slopeEnd = 0.2f;
        mat->altitudeStart = 0.25f;
        mat->altitudeEnd = 0.28f;
    }

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

TextureAsset *getMaterialTexture(EngineApi *engine, AssetHandle assetHandle)
{
    TextureAsset *result = {};

    if (assetHandle)
    {
        LoadedAsset *asset = engine->assetsGetTexture(assetHandle);
        result = asset->texture;
    }

    return result;
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

        uint32 heightmapWidth = 2048;
        uint32 heightmapHeight = 2048;
        uint32 texelsBetweenHorizontalSamples = heightmapWidth / state->heightfield.heightSamplesPerEdge;
        uint32 texelsBetweenVerticalSamples = heightmapHeight / state->heightfield.heightSamplesPerEdge;

        uint16 *src = (uint16 *)heightmapAsset->texture->data;
        float *dst = (float *)state->heightfield.heights;
        float heightScalar = state->heightfield.maxHeight / (float)UINT16_MAX;
        for (uint32 y = 0; y < state->heightfield.heightSamplesPerEdge; y++)
        {
            for (uint32 x = 0; x < state->heightfield.heightSamplesPerEdge; x++)
            {
                *dst++ = *src * heightScalar;
                src += texelsBetweenHorizontalSamples;
            }
            src += (texelsBetweenVerticalSamples - 1) * heightmapWidth;
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
        state->firstPersonCameraPos.y = (state->firstPersonCameraPos.y * 0.95f) + (targetHeight * 0.05f);

        state->firstPersonCameraLookAt = state->firstPersonCameraPos + lookDir;

        // capture mouse if first person camera is active
        memory->platformCaptureMouse();
    }

    for (uint32 layerIdx = 0; layerIdx < MATERIAL_COUNT; layerIdx++)
    {
        RenderTerrainMaterial *mat = &state->materials[layerIdx];
        TerrainMaterialTextures *textures = &state->materialTextures[layerIdx];
        mat->albedoTextureAsset = getMaterialTexture(engine, textures->albedo);
        mat->normalTextureAsset = getMaterialTexture(engine, textures->normal);
        mat->displacementTextureAsset = getMaterialTexture(engine, textures->displacement);
        mat->aoTextureAsset = getMaterialTexture(engine, textures->ao);
    }

    // render world
    float fov = glm::pi<float>() / 4.0f;
    glm::vec3 *cameraPos = state->isOrbitCameraMode ? &state->orbitCameraPos : &state->firstPersonCameraPos;
    glm::vec3 *cameraLookAt =
        state->isOrbitCameraMode ? &state->orbitCameraLookAt : &state->firstPersonCameraLookAt;

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
        state->heightmapTexture, {0}, {0}, {0}, {0}, {0}, {0}, MATERIAL_COUNT, state->materials,
        state->isWireframeMode, 0, glm::vec2(0), 0, 0);
    engine->rendererDrawToScreen(rq, viewport.width, viewport.height);

    endTemporaryMemory(&renderQueueMemory);
}