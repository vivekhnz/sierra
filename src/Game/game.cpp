#include "game.h"

#include "../Engine/engine.h"

#define MAX_PATH 260

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
    rendererUpdateTexture(memory->engineMemory, textureHandle, GL_UNSIGNED_SHORT, GL_R16,
        GL_RED, asset.width, asset.height, asset.data);

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

    if (!rendererInitialize(memory->engineMemory))
    {
        return 0;
    }

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
        rendererCreateBuffer(memory->engineMemory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(memory->engineMemory, state->terrainMeshVertexBufferHandle,
        vertexBufferSize, vertices);
    free(vertices);

    uint32 terrainMeshElementBufferHandle =
        rendererCreateBuffer(memory->engineMemory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
    rendererUpdateBuffer(
        memory->engineMemory, terrainMeshElementBufferHandle, elementBufferSize, indices);
    free(indices);

    state->terrainMeshVertexArrayHandle = rendererCreateVertexArray(memory->engineMemory);
    rendererBindVertexArray(memory->engineMemory, state->terrainMeshVertexArrayHandle);
    rendererBindBuffer(memory->engineMemory, terrainMeshElementBufferHandle);
    rendererBindBuffer(memory->engineMemory, state->terrainMeshVertexBufferHandle);
    rendererBindVertexAttribute(0, GL_FLOAT, false, 3, vertexBufferStride, 0, false);
    rendererBindVertexAttribute(
        1, GL_FLOAT, false, 2, vertexBufferStride, 3 * sizeof(float), false);
    rendererUnbindVertexArray();

    state->terrainMeshTessLevelBufferHandle = rendererCreateBuffer(
        memory->engineMemory, RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
    rendererUpdateBuffer(memory->engineMemory, state->terrainMeshTessLevelBufferHandle,
        state->heightfield.columns * state->heightfield.rows * sizeof(glm::vec4), 0);

    state->heightmapTextureHandle =
        rendererCreateTexture(memory->engineMemory, GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048,
            2048, GL_MIRRORED_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    reloadHeightmap(
        memory, &state->heightfield, state->heightmapTextureHandle, "data/heightmap.tga");

    state->albedoTextureArrayHandle =
        rendererCreateTextureArray(memory->engineMemory, GL_UNSIGNED_BYTE, GL_RGBA, GL_RGB,
            2048, 2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    state->normalTextureArrayHandle =
        rendererCreateTextureArray(memory->engineMemory, GL_UNSIGNED_BYTE, GL_RGB, GL_RGB,
            2048, 2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    state->displacementTextureArrayHandle =
        rendererCreateTextureArray(memory->engineMemory, GL_UNSIGNED_SHORT, GL_R16, GL_RED,
            2048, 2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
    state->aoTextureArrayHandle =
        rendererCreateTextureArray(memory->engineMemory, GL_UNSIGNED_BYTE, GL_R8, GL_RED, 2048,
            2048, MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

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

    state->materialPropsBufferHandle = rendererCreateBuffer(
        memory->engineMemory, RENDERER_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
    rendererUpdateBuffer(memory->engineMemory, state->materialPropsBufferHandle,
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
            memory, &state->heightfield, state->heightmapTextureHandle, "data/heightmap2.tga");
    }

    // toggle terrain wireframe mode when Z is pressed
    if (isNewButtonPress(input, GAME_INPUT_KEY_Z))
    {
        state->isWireframeMode = !state->isWireframeMode;
    }

    if (isLightingStateUpdated)
    {
        rendererUpdateLightingState(memory->engineMemory, &lightDir, state->isLightingEnabled,
            state->isAlbedoEnabled, state->isNormalMapEnabled, state->isAOMapEnabled,
            state->isDisplacementMapEnabled);
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
        float targetHeight = heightfieldGetHeight(&state->heightfield,
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

    rendererUpdateCameraState(memory->engineMemory, &cameraTransform);
    rendererSetViewportSize(viewport.width, viewport.height);
    rendererClearBackBuffer(0.392f, 0.584f, 0.929f, 1);

    TextureAsset *asset;
    asset = engine->assetsGetTexture(memory->engineMemory, ASSET_TEXTURE_GROUND_ALBEDO);
    if (asset && asset->version > state->groundAlbedoTextureVersion)
    {
        rendererUpdateTextureArray(memory->engineMemory, state->albedoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, 0, asset->data);
        state->groundAlbedoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, ASSET_TEXTURE_GROUND_NORMAL);
    if (asset && asset->version > state->groundNormalTextureVersion)
    {
        rendererUpdateTextureArray(memory->engineMemory, state->normalTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, 0, asset->data);
        state->groundNormalTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, ASSET_TEXTURE_GROUND_DISPLACEMENT);
    if (asset && asset->version > state->groundDisplacementTextureVersion)
    {
        rendererUpdateTextureArray(memory->engineMemory, state->displacementTextureArrayHandle,
            GL_UNSIGNED_SHORT, GL_RED, asset->width, asset->height, 0, asset->data);
        state->groundDisplacementTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, ASSET_TEXTURE_GROUND_AO);
    if (asset && asset->version > state->groundAoTextureVersion)
    {
        rendererUpdateTextureArray(memory->engineMemory, state->aoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RED, asset->width, asset->height, 0, asset->data);
        state->groundAoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, ASSET_TEXTURE_ROCK_ALBEDO);
    if (asset && asset->version > state->rockAlbedoTextureVersion)
    {
        rendererUpdateTextureArray(memory->engineMemory, state->albedoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, 1, asset->data);
        state->rockAlbedoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, ASSET_TEXTURE_ROCK_NORMAL);
    if (asset && asset->version > state->rockNormalTextureVersion)
    {
        rendererUpdateTextureArray(memory->engineMemory, state->normalTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, 1, asset->data);
        state->rockNormalTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, ASSET_TEXTURE_ROCK_DISPLACEMENT);
    if (asset && asset->version > state->rockDisplacementTextureVersion)
    {
        rendererUpdateTextureArray(memory->engineMemory, state->displacementTextureArrayHandle,
            GL_UNSIGNED_SHORT, GL_RED, asset->width, asset->height, 1, asset->data);
        state->rockDisplacementTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, ASSET_TEXTURE_ROCK_AO);
    if (asset && asset->version > state->rockAoTextureVersion)
    {
        rendererUpdateTextureArray(memory->engineMemory, state->aoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RED, asset->width, asset->height, 1, asset->data);
        state->rockAoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, ASSET_TEXTURE_SNOW_ALBEDO);
    if (asset && asset->version > state->snowAlbedoTextureVersion)
    {
        rendererUpdateTextureArray(memory->engineMemory, state->albedoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, 2, asset->data);
        state->snowAlbedoTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, ASSET_TEXTURE_SNOW_NORMAL);
    if (asset && asset->version > state->snowNormalTextureVersion)
    {
        rendererUpdateTextureArray(memory->engineMemory, state->normalTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, 2, asset->data);
        state->snowNormalTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, ASSET_TEXTURE_SNOW_DISPLACEMENT);
    if (asset && asset->version > state->snowDisplacementTextureVersion)
    {
        rendererUpdateTextureArray(memory->engineMemory, state->displacementTextureArrayHandle,
            GL_UNSIGNED_SHORT, GL_RED, asset->width, asset->height, 2, asset->data);
        state->snowDisplacementTextureVersion = asset->version;
    }
    asset = engine->assetsGetTexture(memory->engineMemory, ASSET_TEXTURE_SNOW_AO);
    if (asset && asset->version > state->snowAoTextureVersion)
    {
        rendererUpdateTextureArray(memory->engineMemory, state->aoTextureArrayHandle,
            GL_UNSIGNED_BYTE, GL_RED, asset->width, asset->height, 2, asset->data);
        state->snowAoTextureVersion = asset->version;
    }

    uint32 terrainShaderProgramAssetId = state->isWireframeMode
        ? ASSET_SHADER_PROGRAM_TERRAIN_WIREFRAME
        : ASSET_SHADER_PROGRAM_TERRAIN_TEXTURED;
    uint32 terrainPolygonMode = state->isWireframeMode ? GL_LINE : GL_FILL;

    ShaderProgramAsset *calcTessLevelShaderProgram = engine->assetsGetShaderProgram(
        memory->engineMemory, ASSET_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL);
    ShaderProgramAsset *terrainShaderProgram =
        engine->assetsGetShaderProgram(memory->engineMemory, terrainShaderProgramAssetId);
    if (calcTessLevelShaderProgram && terrainShaderProgram)
    {
        uint32 meshEdgeCount = (2 * (state->heightfield.rows * state->heightfield.columns))
            - state->heightfield.rows - state->heightfield.columns;

        rendererSetShaderProgramUniformFloat(memory->engineMemory,
            calcTessLevelShaderProgram->handle, "targetTriangleSize", 0.015f);
        rendererSetShaderProgramUniformInteger(memory->engineMemory,
            calcTessLevelShaderProgram->handle, "horizontalEdgeCount",
            state->heightfield.rows * (state->heightfield.columns - 1));
        rendererSetShaderProgramUniformInteger(memory->engineMemory,
            calcTessLevelShaderProgram->handle, "columnCount", state->heightfield.columns);
        rendererSetShaderProgramUniformFloat(memory->engineMemory,
            calcTessLevelShaderProgram->handle, "terrainHeight", state->heightfield.maxHeight);
        rendererBindTexture(memory->engineMemory, state->heightmapTextureHandle, 0);
        rendererBindTexture(memory->engineMemory, state->heightmapTextureHandle, 5);
        rendererBindShaderStorageBuffer(
            memory->engineMemory, state->terrainMeshTessLevelBufferHandle, 0);
        rendererBindShaderStorageBuffer(
            memory->engineMemory, state->terrainMeshVertexBufferHandle, 1);
        rendererUseShaderProgram(memory->engineMemory, calcTessLevelShaderProgram->handle);
        rendererDispatchCompute(meshEdgeCount, 1, 1);
        rendererShaderStorageMemoryBarrier();

        rendererUseShaderProgram(memory->engineMemory, terrainShaderProgram->handle);
        rendererSetPolygonMode(terrainPolygonMode);
        rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        rendererBindShaderStorageBuffer(
            memory->engineMemory, state->materialPropsBufferHandle, 1);
        rendererSetShaderProgramUniformVector2(memory->engineMemory,
            terrainShaderProgram->handle, "brushHighlightPos", glm::vec2(0.0f, 0.0f));
        rendererSetShaderProgramUniformFloat(memory->engineMemory,
            terrainShaderProgram->handle, "brushHighlightStrength", 0.0f);
        rendererSetShaderProgramUniformFloat(
            memory->engineMemory, terrainShaderProgram->handle, "brushHighlightRadius", 0.0f);
        rendererSetShaderProgramUniformFloat(
            memory->engineMemory, terrainShaderProgram->handle, "brushHighlightFalloff", 0.0f);
        rendererSetShaderProgramUniformVector3(
            memory->engineMemory, terrainShaderProgram->handle, "color", glm::vec3(0, 1, 0));
        rendererSetShaderProgramUniformVector3(memory->engineMemory,
            terrainShaderProgram->handle, "terrainDimensions",
            glm::vec3(state->heightfield.spacing * state->heightfield.columns,
                state->heightfield.maxHeight,
                state->heightfield.spacing * state->heightfield.rows));
        rendererSetShaderProgramUniformInteger(memory->engineMemory,
            terrainShaderProgram->handle, "materialCount", MATERIAL_COUNT);
        rendererBindTexture(memory->engineMemory, state->heightmapTextureHandle, 0);
        rendererBindTextureArray(memory->engineMemory, state->albedoTextureArrayHandle, 1);
        rendererBindTextureArray(memory->engineMemory, state->normalTextureArrayHandle, 2);
        rendererBindTextureArray(
            memory->engineMemory, state->displacementTextureArrayHandle, 3);
        rendererBindTextureArray(memory->engineMemory, state->aoTextureArrayHandle, 4);
        rendererBindTexture(memory->engineMemory, state->heightmapTextureHandle, 5);
        rendererBindVertexArray(memory->engineMemory, state->terrainMeshVertexArrayHandle);
        rendererDrawElements(GL_PATCHES, state->terrainMeshElementCount);
    }
}

API_EXPORT GAME_SHUTDOWN(gameShutdown)
{
    rendererDestroyResources(memory->engineMemory);
}