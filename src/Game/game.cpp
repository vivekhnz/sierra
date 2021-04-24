#include "game.h"

#include "../Engine/engine.h"

#define MAX_PATH 260
#define arrayCount(array) (sizeof(array) / sizeof(array[0]))

float clamp(float value, float min, float max)
{
    return (value < min ? min : (value > max ? max : value));
}

void reloadHeightmap(GameMemory *memory,
    Heightfield *heightfield,
    uint32 textureHandle,
    const char *relativePath)
{
    char absolutePath[MAX_PATH];
    memory->platformGetAssetAbsolutePath(relativePath, absolutePath);
    PlatformReadFileResult result = memory->platformReadFile(absolutePath);
    assert(result.data);

    TextureAsset asset;
    memory->engine.assetsLoadTexture(
        memory->engineMemory, result.data, result.size, true, &asset);
    memory->engine.rendererUpdateTexture(memory->engineMemory, textureHandle,
        GL_UNSIGNED_SHORT, GL_R16, GL_RED, asset.width, asset.height, asset.data);

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

bool initializeGame(GameMemory *memory)
{
    GameState *state = &memory->state;
    GameAssets *assets = &state->assets;
    EngineClientApi *engine = &memory->engine;

    if (!engine->rendererInitialize(memory->engineMemory))
    {
        return 0;
    }

    uint32 wireframeShaderAssetIds[] = {
        ASSET_SHADER_WIREFRAME_VERTEX,    //
        ASSET_SHADER_WIREFRAME_TESS_CTRL, //
        ASSET_SHADER_WIREFRAME_TESS_EVAL, //
        ASSET_SHADER_WIREFRAME_FRAGMENT   //
    };
    assets->shaderProgramTerrainWireframe = engine->assetsRegisterShaderProgram(
        memory->engineMemory, wireframeShaderAssetIds, arrayCount(wireframeShaderAssetIds));

    uint32 texturedShaderAssetIds[] = {
        ASSET_SHADER_TERRAIN_VERTEX,    //
        ASSET_SHADER_TERRAIN_TESS_CTRL, //
        ASSET_SHADER_TERRAIN_TESS_EVAL, //
        ASSET_SHADER_TERRAIN_FRAGMENT   //
    };
    assets->shaderProgramTerrainTextured = engine->assetsRegisterShaderProgram(
        memory->engineMemory, texturedShaderAssetIds, arrayCount(texturedShaderAssetIds));

    uint32 calcTessLevelShaderAssetIds[] = {ASSET_SHADER_TERRAIN_COMPUTE_TESS_LEVEL};
    assets->shaderProgramTerrainCalcTessLevel =
        engine->assetsRegisterShaderProgram(memory->engineMemory, calcTessLevelShaderAssetIds,
            arrayCount(calcTessLevelShaderAssetIds));

    assets->textureGroundAlbedo =
        engine->assetsRegisterTexture(memory->engineMemory, "ground_albedo.bmp", false);
    assets->textureGroundNormal =
        engine->assetsRegisterTexture(memory->engineMemory, "ground_normal.bmp", false);
    assets->textureGroundDisplacement =
        engine->assetsRegisterTexture(memory->engineMemory, "ground_displacement.tga", true);
    assets->textureGroundAo =
        engine->assetsRegisterTexture(memory->engineMemory, "ground_ao.tga", false);
    assets->textureRockAlbedo =
        engine->assetsRegisterTexture(memory->engineMemory, "rock_albedo.jpg", false);
    assets->textureRockNormal =
        engine->assetsRegisterTexture(memory->engineMemory, "rock_normal.jpg", false);
    assets->textureRockDisplacement =
        engine->assetsRegisterTexture(memory->engineMemory, "rock_displacement.tga", true);
    assets->textureRockAo =
        engine->assetsRegisterTexture(memory->engineMemory, "rock_ao.tga", false);
    assets->textureSnowAlbedo =
        engine->assetsRegisterTexture(memory->engineMemory, "snow_albedo.jpg", false);
    assets->textureSnowNormal =
        engine->assetsRegisterTexture(memory->engineMemory, "snow_normal.jpg", false);
    assets->textureSnowDisplacement =
        engine->assetsRegisterTexture(memory->engineMemory, "snow_displacement.tga", true);
    assets->textureSnowAo =
        engine->assetsRegisterTexture(memory->engineMemory, "snow_ao.tga", false);

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

    state->terrainMeshVertexBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(memory->engineMemory, state->terrainMeshVertexBufferHandle,
        vertexBufferSize, vertices);
    free(vertices);

    uint32 terrainMeshElementBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
    engine->rendererUpdateBuffer(
        memory->engineMemory, terrainMeshElementBufferHandle, elementBufferSize, indices);
    free(indices);

    state->terrainMeshVertexArrayHandle =
        engine->rendererCreateVertexArray(memory->engineMemory);
    engine->rendererBindVertexArray(memory->engineMemory, state->terrainMeshVertexArrayHandle);
    engine->rendererBindBuffer(memory->engineMemory, terrainMeshElementBufferHandle);
    engine->rendererBindBuffer(memory->engineMemory, state->terrainMeshVertexBufferHandle);
    engine->rendererBindVertexAttribute(0, GL_FLOAT, false, 3, vertexBufferStride, 0, false);
    engine->rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, vertexBufferStride, 3 * sizeof(float), false);
    engine->rendererUnbindVertexArray();

    state->terrainMeshTessLevelBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
    engine->rendererUpdateBuffer(memory->engineMemory, state->terrainMeshTessLevelBufferHandle,
        state->heightfield.columns * state->heightfield.rows * sizeof(glm::vec4), 0);

    state->heightmapTextureHandle =
        engine->rendererCreateTexture(memory->engineMemory, GL_UNSIGNED_SHORT, GL_R16, GL_RED,
            2048, 2048, GL_MIRRORED_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    reloadHeightmap(
        memory, &state->heightfield, state->heightmapTextureHandle, "heightmap.tga");

    state->albedoTextureArrayHandle =
        engine->rendererCreateTextureArray(memory->engineMemory, GL_UNSIGNED_BYTE, GL_RGBA,
            GL_RGB, 2048, 2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    state->normalTextureArrayHandle =
        engine->rendererCreateTextureArray(memory->engineMemory, GL_UNSIGNED_BYTE, GL_RGB,
            GL_RGB, 2048, 2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    state->displacementTextureArrayHandle =
        engine->rendererCreateTextureArray(memory->engineMemory, GL_UNSIGNED_SHORT, GL_R16,
            GL_RED, 2048, 2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    state->aoTextureArrayHandle =
        engine->rendererCreateTextureArray(memory->engineMemory, GL_UNSIGNED_BYTE, GL_R8,
            GL_RED, 2048, 2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

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

    state->materialPropsBufferHandle = engine->rendererCreateBuffer(
        memory->engineMemory, RENDERER_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
    engine->rendererUpdateBuffer(memory->engineMemory, state->materialPropsBufferHandle,
        sizeof(materialProps), materialProps);

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

    EngineClientApi *engine = &memory->engine;
    GameState *state = &memory->state;
    GameAssets *assets = &state->assets;

    bool isLightingStateUpdated = false;
    glm::vec4 lightDir = glm::vec4(-0.588f, 0.809f, 0.294f, 0.0f);

    if (isButtonDown(input, GAME_INPUT_KEY_ESCAPE))
    {
        memory->platformExitGame();
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
        isLightingStateUpdated = true;
    }

    // toggle albedo texture when T key is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_T))
    {
        state->isAlbedoEnabled = !state->isAlbedoEnabled;
        isLightingStateUpdated = true;
    }

    // toggle normal map texture when N key is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_N))
    {
        state->isNormalMapEnabled = !state->isNormalMapEnabled;
        isLightingStateUpdated = true;
    }

    // toggle displacement map texture when B key is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_B))
    {
        state->isDisplacementMapEnabled = !state->isDisplacementMapEnabled;
        isLightingStateUpdated = true;
    }

    // toggle ambient occlusion texture when O key is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_O))
    {
        state->isAOMapEnabled = !state->isAOMapEnabled;
        isLightingStateUpdated = true;
    }

    // load a different heightmap when H is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_H))
    {
        reloadHeightmap(
            memory, &state->heightfield, state->heightmapTextureHandle, "heightmap2.tga");
    }

    // toggle terrain wireframe mode when Z is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_Z))
    {
        state->isWireframeMode = !state->isWireframeMode;
    }

    if (isLightingStateUpdated)
    {
        engine->rendererUpdateLightingState(memory->engineMemory, &lightDir,
            state->isLightingEnabled, state->isAlbedoEnabled, state->isNormalMapEnabled,
            state->isAOMapEnabled, state->isDisplacementMapEnabled);
        isLightingStateUpdated = false;
    }

    if (state->isOrbitCameraMode)
    {
        bool isManipulatingOrbitCamera = false;

        // orbit distance is modified by scrolling the mouse wheel
        state->orbitCameraDistance *= 1.0f - (glm::sign(input->mouseScrollOffset) * 0.05f);

        // only update the look at position if the middle mouse button is pressed
        if (isButtonDown(input, GAME_INPUT_MOUSE_MIDDLE))
        {
            glm::vec3 lookDir =
                glm::normalize(state->orbitCameraLookAt - state->orbitCameraPos);
            glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
            glm::vec3 yDir = cross(lookDir, xDir);
            glm::vec3 pan =
                (xDir * input->mouseCursorOffset.x) + (yDir * input->mouseCursorOffset.y);
            state->orbitCameraLookAt +=
                pan * clamp(state->orbitCameraDistance, 2.5f, 300.0f) * 0.02f * deltaTime;

            isManipulatingOrbitCamera = true;
        }

        // only update yaw & pitch if the right mouse button is pressed
        if (isButtonDown(input, GAME_INPUT_MOUSE_RIGHT))
        {
            float rotateSensitivity =
                0.05f * clamp(state->orbitCameraDistance, 14.0f, 70.0f) * deltaTime;
            state->orbitCameraYaw +=
                glm::radians(input->mouseCursorOffset.x * rotateSensitivity);
            state->orbitCameraPitch +=
                glm::radians(input->mouseCursorOffset.y * rotateSensitivity);

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
        state->firstPersonCameraPitch = clamp(state->firstPersonCameraPitch
                - ((float)input->mouseCursorOffset.y * lookSensitivity),
            -1.55f, 1.55f);
        glm::vec3 lookDir =
            glm::vec3(cos(state->firstPersonCameraYaw) * cos(state->firstPersonCameraPitch),
                sin(state->firstPersonCameraPitch),
                sin(state->firstPersonCameraYaw) * cos(state->firstPersonCameraPitch));

        // move camera on XZ axis using WASD keys
        glm::vec3 moveDir = glm::vec3(
            cos(state->firstPersonCameraYaw), 0.0f, sin(state->firstPersonCameraYaw));
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
        float targetHeight = engine->heightfieldGetHeight(&state->heightfield,
                                 state->firstPersonCameraPos.x, state->firstPersonCameraPos.z)
            + 1.75f;
        state->firstPersonCameraPos.y =
            (state->firstPersonCameraPos.y * 0.95f) + (targetHeight * 0.05f);

        state->firstPersonCameraLookAt = state->firstPersonCameraPos + lookDir;

        // capture mouse if first person camera is active
        memory->platformCaptureMouse();
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

    engine->rendererUpdateCameraState(memory->engineMemory, &cameraTransform);
    engine->rendererSetViewportSize(viewport.width, viewport.height);
    engine->rendererClearBackBuffer(0.392f, 0.584f, 0.929f, 1);

    LoadedAsset *asset;
    asset = engine->assetsGetTexture(memory->engineMemory, assets->textureGroundAlbedo);
    if (asset->texture && asset->version > state->groundAlbedoTextureVersion)
    {
        engine->rendererUpdateTextureArray(memory->engineMemory,
            state->albedoTextureArrayHandle, GL_UNSIGNED_BYTE, GL_RGB, asset->texture->width,
            asset->texture->height, 0, asset->texture->data);
        state->groundAlbedoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, assets->textureGroundNormal);
    if (asset->texture && asset->version > state->groundNormalTextureVersion)
    {
        engine->rendererUpdateTextureArray(memory->engineMemory,
            state->normalTextureArrayHandle, GL_UNSIGNED_BYTE, GL_RGB, asset->texture->width,
            asset->texture->height, 0, asset->texture->data);
        state->groundNormalTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, assets->textureGroundDisplacement);
    if (asset->texture && asset->version > state->groundDisplacementTextureVersion)
    {
        engine->rendererUpdateTextureArray(memory->engineMemory,
            state->displacementTextureArrayHandle, GL_UNSIGNED_SHORT, GL_RED,
            asset->texture->width, asset->texture->height, 0, asset->texture->data);
        state->groundDisplacementTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, assets->textureGroundAo);
    if (asset->texture && asset->version > state->groundAoTextureVersion)
    {
        engine->rendererUpdateTextureArray(memory->engineMemory, state->aoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RED, asset->texture->width, asset->texture->height, 0,
            asset->texture->data);
        state->groundAoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, assets->textureRockAlbedo);
    if (asset->texture && asset->version > state->rockAlbedoTextureVersion)
    {
        engine->rendererUpdateTextureArray(memory->engineMemory,
            state->albedoTextureArrayHandle, GL_UNSIGNED_BYTE, GL_RGB, asset->texture->width,
            asset->texture->height, 1, asset->texture->data);
        state->rockAlbedoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, assets->textureRockNormal);
    if (asset->texture && asset->version > state->rockNormalTextureVersion)
    {
        engine->rendererUpdateTextureArray(memory->engineMemory,
            state->normalTextureArrayHandle, GL_UNSIGNED_BYTE, GL_RGB, asset->texture->width,
            asset->texture->height, 1, asset->texture->data);
        state->rockNormalTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, assets->textureRockDisplacement);
    if (asset->texture && asset->version > state->rockDisplacementTextureVersion)
    {
        engine->rendererUpdateTextureArray(memory->engineMemory,
            state->displacementTextureArrayHandle, GL_UNSIGNED_SHORT, GL_RED,
            asset->texture->width, asset->texture->height, 1, asset->texture->data);
        state->rockDisplacementTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, assets->textureRockAo);
    if (asset->texture && asset->version > state->rockAoTextureVersion)
    {
        engine->rendererUpdateTextureArray(memory->engineMemory, state->aoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RED, asset->texture->width, asset->texture->height, 1,
            asset->texture->data);
        state->rockAoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, assets->textureSnowAlbedo);
    if (asset->texture && asset->version > state->snowAlbedoTextureVersion)
    {
        engine->rendererUpdateTextureArray(memory->engineMemory,
            state->albedoTextureArrayHandle, GL_UNSIGNED_BYTE, GL_RGB, asset->texture->width,
            asset->texture->height, 2, asset->texture->data);
        state->snowAlbedoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, assets->textureSnowNormal);
    if (asset->texture && asset->version > state->snowNormalTextureVersion)
    {
        engine->rendererUpdateTextureArray(memory->engineMemory,
            state->normalTextureArrayHandle, GL_UNSIGNED_BYTE, GL_RGB, asset->texture->width,
            asset->texture->height, 2, asset->texture->data);
        state->snowNormalTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, assets->textureSnowDisplacement);
    if (asset->texture && asset->version > state->snowDisplacementTextureVersion)
    {
        engine->rendererUpdateTextureArray(memory->engineMemory,
            state->displacementTextureArrayHandle, GL_UNSIGNED_SHORT, GL_RED,
            asset->texture->width, asset->texture->height, 2, asset->texture->data);
        state->snowDisplacementTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, assets->textureSnowAo);
    if (asset->texture && asset->version > state->snowAoTextureVersion)
    {
        engine->rendererUpdateTextureArray(memory->engineMemory, state->aoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RED, asset->texture->width, asset->texture->height, 2,
            asset->texture->data);
        state->snowAoTextureVersion = asset->version;
    }

    uint32 terrainShaderProgramAssetId = state->isWireframeMode
        ? assets->shaderProgramTerrainWireframe
        : assets->shaderProgramTerrainTextured;
    uint32 terrainPolygonMode = state->isWireframeMode ? GL_LINE : GL_FILL;

    LoadedAsset *calcTessLevelShaderProgramAsset = engine->assetsGetShaderProgram(
        memory->engineMemory, assets->shaderProgramTerrainCalcTessLevel);
    LoadedAsset *terrainShaderProgramAsset =
        engine->assetsGetShaderProgram(memory->engineMemory, terrainShaderProgramAssetId);
    ShaderProgramAsset *calcTessLevelShaderProgram =
        calcTessLevelShaderProgramAsset->shaderProgram;
    ShaderProgramAsset *terrainShaderProgram = terrainShaderProgramAsset->shaderProgram;
    if (calcTessLevelShaderProgram && terrainShaderProgram)
    {
        uint32 meshEdgeCount = (2 * (state->heightfield.rows * state->heightfield.columns))
            - state->heightfield.rows - state->heightfield.columns;

        engine->rendererSetShaderProgramUniformFloat(memory->engineMemory,
            calcTessLevelShaderProgram->handle, "targetTriangleSize", 0.015f);
        engine->rendererSetShaderProgramUniformInteger(memory->engineMemory,
            calcTessLevelShaderProgram->handle, "horizontalEdgeCount",
            state->heightfield.rows * (state->heightfield.columns - 1));
        engine->rendererSetShaderProgramUniformInteger(memory->engineMemory,
            calcTessLevelShaderProgram->handle, "columnCount", state->heightfield.columns);
        engine->rendererSetShaderProgramUniformFloat(memory->engineMemory,
            calcTessLevelShaderProgram->handle, "terrainHeight", state->heightfield.maxHeight);
        engine->rendererBindTexture(memory->engineMemory, state->heightmapTextureHandle, 0);
        engine->rendererBindTexture(memory->engineMemory, state->heightmapTextureHandle, 5);
        engine->rendererBindShaderStorageBuffer(
            memory->engineMemory, state->terrainMeshTessLevelBufferHandle, 0);
        engine->rendererBindShaderStorageBuffer(
            memory->engineMemory, state->terrainMeshVertexBufferHandle, 1);
        engine->rendererUseShaderProgram(
            memory->engineMemory, calcTessLevelShaderProgram->handle);
        engine->rendererDispatchCompute(meshEdgeCount, 1, 1);
        engine->rendererShaderStorageMemoryBarrier();

        engine->rendererUseShaderProgram(memory->engineMemory, terrainShaderProgram->handle);
        engine->rendererSetPolygonMode(terrainPolygonMode);
        engine->rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        engine->rendererBindShaderStorageBuffer(
            memory->engineMemory, state->materialPropsBufferHandle, 1);
        engine->rendererSetShaderProgramUniformVector2(memory->engineMemory,
            terrainShaderProgram->handle, "brushHighlightPos", glm::vec2(0.0f, 0.0f));
        engine->rendererSetShaderProgramUniformFloat(memory->engineMemory,
            terrainShaderProgram->handle, "brushHighlightStrength", 0.0f);
        engine->rendererSetShaderProgramUniformFloat(
            memory->engineMemory, terrainShaderProgram->handle, "brushHighlightRadius", 0.0f);
        engine->rendererSetShaderProgramUniformFloat(
            memory->engineMemory, terrainShaderProgram->handle, "brushHighlightFalloff", 0.0f);
        engine->rendererSetShaderProgramUniformVector3(
            memory->engineMemory, terrainShaderProgram->handle, "color", glm::vec3(0, 1, 0));
        engine->rendererSetShaderProgramUniformVector3(memory->engineMemory,
            terrainShaderProgram->handle, "terrainDimensions",
            glm::vec3(state->heightfield.spacing * state->heightfield.columns,
                state->heightfield.maxHeight,
                state->heightfield.spacing * state->heightfield.rows));
        engine->rendererSetShaderProgramUniformInteger(memory->engineMemory,
            terrainShaderProgram->handle, "materialCount", MATERIAL_COUNT);
        engine->rendererBindTexture(memory->engineMemory, state->heightmapTextureHandle, 0);
        engine->rendererBindTextureArray(
            memory->engineMemory, state->albedoTextureArrayHandle, 1);
        engine->rendererBindTextureArray(
            memory->engineMemory, state->normalTextureArrayHandle, 2);
        engine->rendererBindTextureArray(
            memory->engineMemory, state->displacementTextureArrayHandle, 3);
        engine->rendererBindTextureArray(memory->engineMemory, state->aoTextureArrayHandle, 4);
        engine->rendererBindTexture(memory->engineMemory, state->heightmapTextureHandle, 5);
        engine->rendererBindVertexArray(
            memory->engineMemory, state->terrainMeshVertexArrayHandle);
        engine->rendererDrawElements(GL_PATCHES, state->terrainMeshElementCount);
    }
}

API_EXPORT GAME_SHUTDOWN(gameShutdown)
{
    memory->engine.rendererDestroyResources(memory->engineMemory);
}