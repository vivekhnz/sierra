#include "SceneWorld.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "../../Engine/terrain_assets.h"
#include "../../Engine/IO/MouseButton.hpp"
#include "../../Engine/IO/Key.hpp"

bool isMouseButtonDown(EditorInput *input, Terrain::Engine::IO::MouseButton button)
{
    return input->pressedMouseButtons & static_cast<uint8>(button);
}

bool isNewMouseButtonPress(EditorInput *input, Terrain::Engine::IO::MouseButton button)
{
    uint8 buttonVal = static_cast<uint8>(button);
    return (input->pressedMouseButtons & buttonVal)
        && !(input->prevPressedMouseButtons & buttonVal);
}

bool isKeyDown(EditorInput *input, Terrain::Engine::IO::Key key)
{
    return input->pressedKeys & static_cast<uint64>(key);
}

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    SceneWorld::SceneWorld(EngineMemory *memory) : memory(memory)
    {
        // allocate a buffer that heightmap texture data can be copied into
        heightmapTextureDataTempBuffer = malloc(2048 * 2048 * 2);
    }

    SceneWorld::~SceneWorld()
    {
        free(heightmapTextureDataTempBuffer);
    }

    void SceneWorld::initialize(uint32 heightmapTextureHandle, uint32 previewTextureHandle)
    {
        heightfield = {};
        heightfield.columns = HEIGHTFIELD_COLUMNS;
        heightfield.rows = HEIGHTFIELD_ROWS;
        heightfield.spacing = 0.5f;
        heightfield.maxHeight = 25.0f;
        heightfield.position = glm::vec2(-63.75f, -63.75f);
        heightfield.heights = heightfieldHeights;

        this->heightmapTextureHandle = heightmapTextureHandle;
        this->previewTextureHandle = previewTextureHandle;

        // create terrain mesh
        terrainMesh = {};
        terrainMesh.elementCount = (heightfield.rows - 1) * (heightfield.columns - 1) * 4;

        uint32 vertexBufferStride = 5 * sizeof(float);
        uint32 vertexBufferSize = heightfield.columns * heightfield.rows * vertexBufferStride;
        float *vertices = (float *)malloc(vertexBufferSize);

        uint32 elementBufferSize = sizeof(uint32) * terrainMesh.elementCount;
        uint32 *indices = (uint32 *)malloc(elementBufferSize);

        float offsetX = (heightfield.columns - 1) * heightfield.spacing * -0.5f;
        float offsetY = (heightfield.rows - 1) * heightfield.spacing * -0.5f;
        glm::vec2 uvSize =
            glm::vec2(1.0f / (heightfield.columns - 1), 1.0f / (heightfield.rows - 1));

        float *currentVertex = vertices;
        uint32 *currentIndex = indices;
        for (uint32 y = 0; y < heightfield.rows; y++)
        {
            for (uint32 x = 0; x < heightfield.columns; x++)
            {
                *currentVertex++ = (x * heightfield.spacing) + offsetX;
                *currentVertex++ = 0;
                *currentVertex++ = (y * heightfield.spacing) + offsetY;
                *currentVertex++ = uvSize.x * x;
                *currentVertex++ = uvSize.y * y;

                if (y < heightfield.rows - 1 && x < heightfield.columns - 1)
                {
                    uint32 patchIndex = (y * heightfield.columns) + x;
                    *currentIndex++ = patchIndex;
                    *currentIndex++ = patchIndex + heightfield.columns;
                    *currentIndex++ = patchIndex + heightfield.columns + 1;
                    *currentIndex++ = patchIndex + 1;
                }
            }
        }

        terrainMesh.vertexBufferHandle =
            rendererCreateBuffer(memory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
        rendererUpdateBuffer(
            memory, terrainMesh.vertexBufferHandle, vertexBufferSize, vertices);
        free(vertices);

        uint32 elementBufferHandle =
            rendererCreateBuffer(memory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
        rendererUpdateBuffer(memory, elementBufferHandle, elementBufferSize, indices);
        free(indices);

        terrainMesh.vertexArrayHandle = rendererCreateVertexArray(memory);
        rendererBindVertexArray(memory, terrainMesh.vertexArrayHandle);
        rendererBindBuffer(memory, elementBufferHandle);
        rendererBindBuffer(memory, terrainMesh.vertexBufferHandle);
        rendererBindVertexAttribute(0, GL_FLOAT, false, 3, vertexBufferStride, 0, false);
        rendererBindVertexAttribute(
            1, GL_FLOAT, false, 2, vertexBufferStride, 3 * sizeof(float), false);
        rendererUnbindVertexArray();

        // create buffer to store vertex edge data
        tessellationLevelBufferHandle =
            rendererCreateBuffer(memory, RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
        rendererUpdateBuffer(memory, tessellationLevelBufferHandle,
            heightfield.columns * heightfield.rows * sizeof(glm::vec4), 0);

        albedoTextureArrayHandle = rendererCreateTextureArray(memory, GL_UNSIGNED_BYTE, GL_RGB,
            GL_RGB, 2048, 2048, MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        normalTextureArrayHandle = rendererCreateTextureArray(memory, GL_UNSIGNED_BYTE, GL_RGB,
            GL_RGB, 2048, 2048, MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        displacementTextureArrayHandle =
            rendererCreateTextureArray(memory, GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048, 2048,
                MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        aoTextureArrayHandle = rendererCreateTextureArray(memory, GL_UNSIGNED_BYTE, GL_R8,
            GL_RED, 2048, 2048, MAX_MATERIAL_COUNT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

        worldState.materialCount = 0;
        for (uint32 i = 0; i < MAX_MATERIAL_COUNT; i++)
        {
            worldState.materialProps[i] = {};
            worldState.albedoTextureAssetIds[i] = {};
            worldState.normalTextureAssetIds[i] = {};
            worldState.displacementTextureAssetIds[i] = {};
            worldState.aoTextureAssetIds[i] = {};

            albedoTextures[i] = {};
            normalTextures[i] = {};
            displacementTextures[i] = {};
            aoTextures[i] = {};
        }
        materialPropsBufferHandle =
            rendererCreateBuffer(memory, RENDERER_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
        rendererUpdateBuffer(
            memory, materialPropsBufferHandle, sizeof(worldState.materialProps), 0);
    }

    void *SceneWorld::addView()
    {
        assert(viewStateCount < MAX_SCENE_VIEWS);

        ViewState *state = &viewStates[viewStateCount++];
        state->orbitCameraDistance = 112.5f;
        state->orbitCameraYaw = glm::radians(180.0f);
        state->orbitCameraPitch = glm::radians(15.0f);
        state->cameraLookAt = glm::vec3(0, 0, 0);

        glm::vec3 lookDir =
            glm::vec3(cos(state->orbitCameraYaw) * cos(state->orbitCameraPitch),
                sin(state->orbitCameraPitch),
                sin(state->orbitCameraYaw) * cos(state->orbitCameraPitch));
        state->cameraPos = state->cameraLookAt + (lookDir * state->orbitCameraDistance);

        return state;
    }

    void SceneWorld::update(EditorMemory *editorMemory, float deltaTime, EditorInput *input)
    {
        EditorState *state = &editorMemory->currentState;
        EditorState *newState = &editorMemory->newState;

        if (state->heightmapStatus != HEIGHTMAP_STATUS_IDLE)
        {
            // update heightfield with composited heightmap texture
            rendererReadTexturePixels(memory, heightmapTextureHandle, GL_UNSIGNED_SHORT,
                GL_RED, heightmapTextureDataTempBuffer);

            uint16 heightmapWidth = 2048;
            uint16 heightmapHeight = 2048;
            uint16 patchTexelWidth = heightmapWidth / heightfield.columns;
            uint16 patchTexelHeight = heightmapHeight / heightfield.rows;

            uint16 *src = (uint16 *)heightmapTextureDataTempBuffer;
            float *dst = (float *)heightfieldHeights;
            float heightScalar = heightfield.maxHeight / (float)UINT16_MAX;
            for (uint32 y = 0; y < heightfield.rows; y++)
            {
                for (uint32 x = 0; x < heightfield.columns; x++)
                {
                    *dst++ = *src * heightScalar;
                    src += patchTexelWidth;
                }
                src += (patchTexelHeight - 1) * heightmapWidth;
            }
        }

        bool isManipulatingCamera = false;
        if (input->activeViewState)
        {
            ViewState *viewState = (ViewState *)input->activeViewState;
            isManipulatingCamera = updateViewState(viewState, deltaTime, input);
        }
        if (isManipulatingCamera)
        {
            editorMemory->platformCaptureMouse(false);
        }

        // determine the current operation being performed
        OperationState operation = getCurrentOperation(state, input);

        // update editor state
        newState->mode = operation.mode;
        newState->tool = operation.tool;
        newState->heightmapStatus = getNextHeightmapStatus(
            state->heightmapStatus, operation.isBrushActive, operation.isDiscardingStroke);
        newState->currentBrushPos = operation.brushPosition;

        worldState.isPreviewingChanges = operation.isBrushActive;
        if (operation.mode == INTERACTION_MODE_MODIFY_BRUSH_RADIUS)
        {
            editorMemory->platformCaptureMouse(true);
            newState->brushRadius =
                glm::clamp(state->brushRadius + operation.brushRadiusIncrease, 32.0f, 2048.0f);
            worldState.isPreviewingChanges = true;
        }
        else if (operation.mode == INTERACTION_MODE_MODIFY_BRUSH_FALLOFF)
        {
            editorMemory->platformCaptureMouse(true);
            newState->brushFalloff =
                glm::clamp(state->brushFalloff + operation.brushFalloffIncrease, 0.0f, 0.99f);
            worldState.isPreviewingChanges = true;
        }
        else if (operation.mode == INTERACTION_MODE_MODIFY_BRUSH_STRENGTH)
        {
            editorMemory->platformCaptureMouse(true);
            newState->brushStrength = glm::clamp(
                state->brushStrength + operation.brushStrengthIncrease, 0.01f, 1.0f);
            worldState.isPreviewingChanges = true;
        }

        // update brush highlight
        worldState.brushPos = operation.brushPosition;
        worldState.isBrushHighlightVisible = operation.mode != INTERACTION_MODE_MOVE_CAMERA;
        worldState.brushRadius = state->brushRadius / 2048.0f;
        worldState.brushFalloff = state->brushFalloff;

        // update material properties
        worldState.materialCount = state->materialCount;
        for (uint32 i = 0; i < worldState.materialCount; i++)
        {
            const MaterialProperties *stateProps = &state->materialProps[i];
            GpuMaterialProperties *gpuProps = &worldState.materialProps[i];

            gpuProps->textureSizeInWorldUnits.x = stateProps->textureSizeInWorldUnits;
            gpuProps->textureSizeInWorldUnits.y = stateProps->textureSizeInWorldUnits;
            gpuProps->rampParams.x = stateProps->slopeStart;
            gpuProps->rampParams.y = stateProps->slopeEnd;
            gpuProps->rampParams.z = stateProps->altitudeStart;
            gpuProps->rampParams.w = stateProps->altitudeEnd;

            worldState.albedoTextureAssetIds[i] = stateProps->albedoTextureAssetId;
            worldState.normalTextureAssetIds[i] = stateProps->normalTextureAssetId;
            worldState.displacementTextureAssetIds[i] = stateProps->displacementTextureAssetId;
            worldState.aoTextureAssetIds[i] = stateProps->aoTextureAssetId;
        }

        // update scene lighting
        glm::vec4 lightDir = glm::vec4(0);
        lightDir.x = sin(state->lightDirection * glm::pi<float>() * -0.5);
        lightDir.y = cos(state->lightDirection * glm::pi<float>() * 0.5);
        lightDir.z = 0.2f;
        rendererUpdateLightingState(memory, &lightDir, true, true, true, true, true);
    }

    bool SceneWorld::updateViewState(ViewState *viewState, float deltaTime, EditorInput *input)
    {
        bool isManipulatingCamera = false;

        // orbit distance is modified by scrolling the mouse wheel
        viewState->orbitCameraDistance *= 1.0f - (glm::sign(input->scrollOffset) * 0.05f);

        if (isMouseButtonDown(input, IO::MouseButton::Middle))
        {
            // update the look at position if the middle mouse button is pressed
            glm::vec3 lookDir = glm::normalize(viewState->cameraLookAt - viewState->cameraPos);
            glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
            glm::vec3 yDir = cross(lookDir, xDir);
            glm::vec3 pan = (xDir * input->cursorOffset.x) + (yDir * input->cursorOffset.y);
            float panMagnitude = glm::clamp(viewState->orbitCameraDistance, 2.5f, 300.0f);
            viewState->cameraLookAt += pan * panMagnitude * 0.02f * deltaTime;

            isManipulatingCamera = true;
        }
        if (isMouseButtonDown(input, IO::MouseButton::Right))
        {
            // update yaw & pitch if the right mouse button is pressed
            float rotateMagnitude = glm::clamp(viewState->orbitCameraDistance, 14.0f, 70.0f);
            float rotateSensitivity = 0.05f * rotateMagnitude * deltaTime;
            viewState->orbitCameraYaw +=
                glm::radians(input->cursorOffset.x * rotateSensitivity);
            viewState->orbitCameraPitch +=
                glm::radians(input->cursorOffset.y * rotateSensitivity);

            isManipulatingCamera = true;
        }

        // calculate camera position
        glm::vec3 newLookDir =
            glm::vec3(cos(viewState->orbitCameraYaw) * cos(viewState->orbitCameraPitch),
                sin(viewState->orbitCameraPitch),
                sin(viewState->orbitCameraYaw) * cos(viewState->orbitCameraPitch));
        viewState->cameraPos =
            viewState->cameraLookAt + (newLookDir * viewState->orbitCameraDistance);

        return isManipulatingCamera;
    }

    SceneWorld::OperationState SceneWorld::getCurrentOperation(
        EditorState *prevState, EditorInput *input)
    {
        EditorTool tool = prevState->tool;

        ViewState *activeViewState = (ViewState *)input->activeViewState;
        if (activeViewState)
        {
            if (isMouseButtonDown(input, IO::MouseButton::Middle)
                || isMouseButtonDown(input, IO::MouseButton::Right))
            {
                OperationState op = {};
                op.mode = INTERACTION_MODE_MOVE_CAMERA;
                op.tool = tool;
                op.isBrushActive = false;
                op.isDiscardingStroke = false;
                op.brushPosition = glm::vec2();
                op.brushRadiusIncrease = 0.0f;
                op.brushFalloffIncrease = 0.0f;
                op.brushStrengthIncrease = 0.0f;
                return op;
            }
            if (prevState->heightmapStatus == HEIGHTMAP_STATUS_EDITING
                && isKeyDown(input, IO::Key::Escape))
            {
                OperationState op = {};
                op.mode = INTERACTION_MODE_PAINT_BRUSH_STROKE;
                op.tool = tool;
                op.isBrushActive = false;
                op.isDiscardingStroke = true;
                op.brushPosition = glm::vec2();
                op.brushRadiusIncrease = 0.0f;
                op.brushFalloffIncrease = 0.0f;
                op.brushStrengthIncrease = 0.0f;
                return op;
            }

            glm::vec2 mousePos = (input->normalizedCursorPos * 2.0f) - 1.0f;
            glm::mat4 inverseViewProjection = glm::inverse(activeViewState->cameraTransform);
            glm::vec4 screenPos = glm::vec4(mousePos.x, -mousePos.y, 1.0f, 1.0f);
            glm::vec4 worldPos = inverseViewProjection * screenPos;

            glm::vec3 intersectionPoint;
            if (heightfieldIsRayIntersecting(&heightfield, activeViewState->cameraPos,
                    glm::normalize(glm::vec3(worldPos)), intersectionPoint))
            {
                glm::vec2 normalizedPickPoint =
                    glm::vec2((intersectionPoint.x / 127.5f) + 0.5f,
                        (intersectionPoint.z / 127.5f) + 0.5f);

                // if the R key is pressed, we are adjusting the brush radius
                if (isKeyDown(input, IO::Key::R))
                {
                    float brushRadiusIncrease = input->cursorOffset.x + input->cursorOffset.y;

                    OperationState op = {};
                    op.mode = INTERACTION_MODE_MODIFY_BRUSH_RADIUS;
                    op.tool = tool;
                    op.isBrushActive = false;
                    op.isDiscardingStroke = false;
                    op.brushPosition = normalizedPickPoint;
                    op.brushRadiusIncrease = brushRadiusIncrease;
                    op.brushFalloffIncrease = 0.0f;
                    op.brushStrengthIncrease = 0.0f;
                    return op;
                }

                // if the F key is pressed, we are adjusting the brush falloff
                if (isKeyDown(input, IO::Key::F))
                {
                    float brushFalloffIncrease =
                        (input->cursorOffset.x + input->cursorOffset.y) * 0.001f;

                    OperationState op = {};
                    op.mode = INTERACTION_MODE_MODIFY_BRUSH_FALLOFF;
                    op.tool = tool;
                    op.isBrushActive = false;
                    op.isDiscardingStroke = false;
                    op.brushPosition = normalizedPickPoint;
                    op.brushRadiusIncrease = 0.0f;
                    op.brushFalloffIncrease = brushFalloffIncrease;
                    op.brushStrengthIncrease = 0.0f;
                    return op;
                }

                // if the S key is pressed, we are adjusting the brush strength
                if (isKeyDown(input, IO::Key::S))
                {
                    float brushStrengthIncrease =
                        (input->cursorOffset.x + input->cursorOffset.y) * 0.001f;

                    OperationState op = {};
                    op.mode = INTERACTION_MODE_MODIFY_BRUSH_STRENGTH;
                    op.tool = tool;
                    op.isBrushActive = false;
                    op.isDiscardingStroke = false;
                    op.brushPosition = normalizedPickPoint;
                    op.brushRadiusIncrease = 0.0f;
                    op.brushFalloffIncrease = 0.0f;
                    op.brushStrengthIncrease = brushStrengthIncrease;
                    return op;
                }

                // if a number key is pressed, change the selected tool
                if (isKeyDown(input, IO::Key::D1))
                {
                    tool = EDITOR_TOOL_RAISE_TERRAIN;
                }
                else if (isKeyDown(input, IO::Key::D2))
                {
                    tool = EDITOR_TOOL_LOWER_TERRAIN;
                }

                // the LMB must be newly pressed to start a new brush stroke
                bool isBrushActive = false;
                if (prevState->heightmapStatus == HEIGHTMAP_STATUS_EDITING
                    && isMouseButtonDown(input, IO::MouseButton::Left))
                {
                    isBrushActive = true;
                }
                else if (isNewMouseButtonPress(input, IO::MouseButton::Left))
                {
                    isBrushActive = true;
                }

                OperationState op = {};
                op.mode = INTERACTION_MODE_PAINT_BRUSH_STROKE;
                op.tool = tool;
                op.isBrushActive = isBrushActive;
                op.isDiscardingStroke = false;
                op.brushPosition = normalizedPickPoint;
                op.brushRadiusIncrease = 0.0f;
                op.brushFalloffIncrease = 0.0f;
                op.brushStrengthIncrease = 0.0f;
                return op;
            }
        }

        OperationState op = {};
        op.mode = INTERACTION_MODE_PAINT_BRUSH_STROKE;
        op.tool = tool;
        op.isBrushActive = false;
        op.isDiscardingStroke = false;
        op.brushPosition = glm::vec2(-10000, -10000);
        op.brushRadiusIncrease = 0.0f;
        op.brushFalloffIncrease = 0.0f;
        op.brushStrengthIncrease = 0.0f;
        return op;
    }

    HeightmapStatus SceneWorld::getNextHeightmapStatus(
        HeightmapStatus currentHeightmapStatus, bool isBrushActive, bool isDiscardingStroke)
    {
        switch (currentHeightmapStatus)
        {
        case HEIGHTMAP_STATUS_COMMITTING:
        case HEIGHTMAP_STATUS_IDLE:
            return isBrushActive ? HEIGHTMAP_STATUS_EDITING : HEIGHTMAP_STATUS_IDLE;
        case HEIGHTMAP_STATUS_EDITING:
            return isDiscardingStroke
                ? HEIGHTMAP_STATUS_DISCARDING
                : (isBrushActive ? HEIGHTMAP_STATUS_EDITING : HEIGHTMAP_STATUS_COMMITTING);
        case HEIGHTMAP_STATUS_DISCARDING:
            return HEIGHTMAP_STATUS_COMMITTING;
        }

        return currentHeightmapStatus;
    }

    void SceneWorld::render(EditorMemory *memory, EditorViewContext *view)
    {
        ViewState *viewState = (ViewState *)view->viewState;

        // calculate camera transform
        constexpr float fov = glm::pi<float>() / 4.0f;
        const float nearPlane = 0.1f;
        const float farPlane = 10000.0f;
        const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        const float aspectRatio = (float)view->width / (float)view->height;
        glm::mat4 projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
        viewState->cameraTransform =
            projection * glm::lookAt(viewState->cameraPos, viewState->cameraLookAt, up);

        rendererUpdateCameraState(&memory->engine, &viewState->cameraTransform);
        rendererSetViewportSize(view->width, view->height);
        rendererClearBackBuffer(0.392f, 0.584f, 0.929f, 1);

        // get textures
        for (uint32 layerIdx = 0; layerIdx < worldState.materialCount; layerIdx++)
        {
            uint32 assetId;
            TextureAsset *asset;
            TextureAssetBinding *binding;

            assetId = worldState.albedoTextureAssetIds[layerIdx];
            if (assetId)
            {
                binding = &albedoTextures[layerIdx];
                asset = assetsGetTexture(&memory->engine, assetId);
                if (asset
                    && (assetId != binding->assetId || asset->version > binding->version))
                {
                    rendererUpdateTextureArray(&memory->engine, albedoTextureArrayHandle,
                        GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, layerIdx,
                        asset->data);
                    binding->assetId = assetId;
                    binding->version = asset->version;
                }
            }

            assetId = worldState.normalTextureAssetIds[layerIdx];
            if (assetId)
            {
                binding = &normalTextures[layerIdx];
                asset = assetsGetTexture(&memory->engine, assetId);
                if (asset
                    && (assetId != binding->assetId || asset->version > binding->version))
                {
                    rendererUpdateTextureArray(&memory->engine, normalTextureArrayHandle,
                        GL_UNSIGNED_BYTE, GL_RGB, asset->width, asset->height, layerIdx,
                        asset->data);
                    binding->assetId = assetId;
                    binding->version = asset->version;
                }
            }

            assetId = worldState.displacementTextureAssetIds[layerIdx];
            if (assetId)
            {
                binding = &displacementTextures[layerIdx];
                asset = assetsGetTexture(&memory->engine, assetId);
                if (asset
                    && (assetId != binding->assetId || asset->version > binding->version))
                {
                    rendererUpdateTextureArray(&memory->engine, displacementTextureArrayHandle,
                        GL_UNSIGNED_SHORT, GL_RED, asset->width, asset->height, layerIdx,
                        asset->data);
                    binding->assetId = assetId;
                    binding->version = asset->version;
                }
            }

            assetId = worldState.aoTextureAssetIds[layerIdx];
            if (assetId)
            {
                binding = &aoTextures[layerIdx];
                asset = assetsGetTexture(&memory->engine, assetId);
                if (asset
                    && (assetId != binding->assetId || asset->version > binding->version))
                {
                    rendererUpdateTextureArray(&memory->engine, aoTextureArrayHandle,
                        GL_UNSIGNED_BYTE, GL_RED, asset->width, asset->height, layerIdx,
                        asset->data);
                    binding->assetId = assetId;
                    binding->version = asset->version;
                }
            }
        }

        // get shader programs
        ShaderProgramAsset *calcTessLevelShaderProgramAsset = assetsGetShaderProgram(
            &memory->engine, ASSET_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL);
        ShaderProgramAsset *terrainShaderProgramAsset =
            assetsGetShaderProgram(&memory->engine, ASSET_SHADER_PROGRAM_TERRAIN_TEXTURED);
        if (!calcTessLevelShaderProgramAsset || !terrainShaderProgramAsset)
            return;

        // calculate tessellation levels
        uint32 calcTessLevelShaderProgramHandle = calcTessLevelShaderProgramAsset->handle;
        rendererSetShaderProgramUniformFloat(
            &memory->engine, calcTessLevelShaderProgramHandle, "targetTriangleSize", 0.015f);

        rendererSetShaderProgramUniformInteger(&memory->engine,
            calcTessLevelShaderProgramHandle, "horizontalEdgeCount",
            heightfield.rows * (heightfield.columns - 1));
        rendererSetShaderProgramUniformInteger(&memory->engine,
            calcTessLevelShaderProgramHandle, "columnCount", heightfield.columns);
        rendererSetShaderProgramUniformFloat(&memory->engine, calcTessLevelShaderProgramHandle,
            "terrainHeight", heightfield.maxHeight);
        rendererBindTexture(&memory->engine, heightmapTextureHandle, 0);
        rendererBindTexture(&memory->engine, previewTextureHandle, 5);

        uint32 meshEdgeCount = (2 * (heightfield.rows * heightfield.columns))
            - heightfield.rows - heightfield.columns;

        rendererBindShaderStorageBuffer(&memory->engine, tessellationLevelBufferHandle, 0);
        rendererBindShaderStorageBuffer(&memory->engine, terrainMesh.vertexBufferHandle, 1);
        rendererUseShaderProgram(&memory->engine, calcTessLevelShaderProgramHandle);
        rendererDispatchCompute(meshEdgeCount, 1, 1);
        rendererShaderStorageMemoryBarrier();

        // bind material data
        rendererUpdateBuffer(&memory->engine, materialPropsBufferHandle,
            sizeof(worldState.materialProps), worldState.materialProps);

        uint32 terrainShaderProgramHandle = terrainShaderProgramAsset->handle;
        rendererUseShaderProgram(&memory->engine, terrainShaderProgramHandle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        rendererBindTexture(&memory->engine, heightmapTextureHandle, 0);
        rendererBindTextureArray(&memory->engine, albedoTextureArrayHandle, 1);
        rendererBindTextureArray(&memory->engine, normalTextureArrayHandle, 2);
        rendererBindTextureArray(&memory->engine, displacementTextureArrayHandle, 3);
        rendererBindTextureArray(&memory->engine, aoTextureArrayHandle, 4);
        rendererBindTexture(&memory->engine,
            worldState.isPreviewingChanges ? previewTextureHandle : heightmapTextureHandle, 5);
        rendererBindShaderStorageBuffer(&memory->engine, materialPropsBufferHandle, 1);

        // bind mesh data
        rendererBindVertexArray(&memory->engine, terrainMesh.vertexArrayHandle);

        // set shader uniforms
        rendererSetShaderProgramUniformInteger(&memory->engine, terrainShaderProgramHandle,
            "materialCount", worldState.materialCount);
        rendererSetShaderProgramUniformVector3(&memory->engine, terrainShaderProgramHandle,
            "terrainDimensions",
            glm::vec3(heightfield.spacing * heightfield.columns, heightfield.maxHeight,
                heightfield.spacing * heightfield.rows));
        rendererSetShaderProgramUniformFloat(&memory->engine, terrainShaderProgramHandle,
            "brushHighlightStrength", worldState.isBrushHighlightVisible ? 1 : 0);
        rendererSetShaderProgramUniformVector2(&memory->engine, terrainShaderProgramHandle,
            "brushHighlightPos", worldState.brushPos);
        rendererSetShaderProgramUniformFloat(&memory->engine, terrainShaderProgramHandle,
            "brushHighlightRadius", worldState.brushRadius);
        rendererSetShaderProgramUniformFloat(&memory->engine, terrainShaderProgramHandle,
            "brushHighlightFalloff", worldState.brushFalloff);

        // draw mesh
        rendererDrawElements(GL_PATCHES, terrainMesh.elementCount);
    }
}}}}