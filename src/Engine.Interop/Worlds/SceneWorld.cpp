#include "SceneWorld.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "../../Engine/terrain_assets.h"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    SceneWorld::SceneWorld(EngineContext &ctx) : ctx(ctx), world(ctx)
    {
        // allocate a buffer that heightmap texture data can be copied into
        heightmapTextureDataTempBuffer = malloc(2048 * 2048 * 2);
    }

    SceneWorld::~SceneWorld()
    {
        free(heightmapTextureDataTempBuffer);
    }

    void SceneWorld::initialize(uint32 heightmapTextureHandle)
    {
        heightfield = {};
        heightfield.columns = HEIGHTFIELD_COLUMNS;
        heightfield.rows = HEIGHTFIELD_ROWS;
        heightfield.spacing = 0.5f;
        heightfield.maxHeight = 25.0f;
        heightfield.position = glm::vec2(-63.75f, -63.75f);
        heightfield.heights = heightfieldHeights;

        this->heightmapTextureHandle = heightmapTextureHandle;

        // create entity and components
        int entityId = ctx.entities.create();
        int terrainRendererInstanceId = world.componentManagers.terrainRenderer.create(
            entityId, heightfield.rows, heightfield.columns, heightfield.spacing);
        meshHandle =
            world.componentManagers.terrainRenderer.getMeshHandle(terrainRendererInstanceId);

        // create buffer to store vertex edge data
        tessellationLevelBufferHandle =
            rendererCreateBuffer(ctx.memory, RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
        rendererUpdateBuffer(ctx.memory, tessellationLevelBufferHandle,
            heightfield.columns * heightfield.rows * sizeof(glm::vec4), 0);

        albedoTextureArrayHandle = rendererCreateTextureArray(ctx.memory, GL_UNSIGNED_BYTE,
            GL_RGB, GL_RGB, 2048, 2048, 3, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        normalTextureArrayHandle = rendererCreateTextureArray(ctx.memory, GL_UNSIGNED_BYTE,
            GL_RGB, GL_RGB, 2048, 2048, 3, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        displacementTextureArrayHandle =
            rendererCreateTextureArray(ctx.memory, GL_UNSIGNED_SHORT, GL_R16, GL_RED, 2048,
                2048, 3, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);
        aoTextureArrayHandle = rendererCreateTextureArray(ctx.memory, GL_UNSIGNED_BYTE, GL_R8,
            GL_RED, 2048, 2048, 3, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR);

        for (uint32 i = 0; i < MATERIAL_COUNT; i++)
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
            rendererCreateBuffer(ctx.memory, RENDERER_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
        rendererUpdateBuffer(
            ctx.memory, materialPropsBufferHandle, sizeof(worldState.materialProps), 0);
    }

    void SceneWorld::linkViewport(ViewportContext &vctx)
    {
        assert(viewStateCount < MAX_SCENE_VIEWS);

        vctx.setCameraEntityId(viewStateCount);

        ViewState *state = &viewStates[viewStateCount++];
        state->inputControllerId = vctx.getInputControllerId();
        state->orbitCameraDistance = 112.5f;
        state->orbitCameraYaw = glm::radians(180.0f);
        state->orbitCameraPitch = glm::radians(15.0f);
        state->cameraPos = glm::vec3(0, 0, 0);
        state->cameraLookAt = glm::vec3(0, 0, 0);
    }

    void SceneWorld::update(float deltaTime, const EditorState &state, EditorState &newState)
    {
        if (state.heightmapStatus != HeightmapStatus::Idle)
        {
            // update heightfield with composited heightmap texture
            rendererReadTexturePixels(ctx.memory, heightmapTextureHandle, GL_UNSIGNED_SHORT,
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
        for (int i = 0; i < viewStateCount; i++)
        {
            isManipulatingCamera |= updateViewState(&viewStates[i], deltaTime);
        }
        if (isManipulatingCamera)
        {
            ctx.input.captureMouse(false);
        }

        // determine the current operation being performed
        OperationState operation = getCurrentOperation(state);

        // update editor state
        newState.mode = operation.mode;
        newState.tool = operation.tool;
        newState.heightmapStatus = getNextHeightmapStatus(
            state.heightmapStatus, operation.isBrushActive, operation.isDiscardingStroke);
        newState.currentBrushPos = operation.brushPosition;

        if (operation.mode == InteractionMode::ModifyBrushRadius)
        {
            ctx.input.captureMouse(true);
            newState.brushRadius =
                glm::clamp(state.brushRadius + operation.brushRadiusIncrease, 32.0f, 512.0f);
        }
        else if (operation.mode == InteractionMode::ModifyBrushFalloff)
        {
            ctx.input.captureMouse(true);
            newState.brushFalloff =
                glm::clamp(state.brushFalloff + operation.brushFalloffIncrease, 0.0f, 0.99f);
        }

        // update brush highlight
        worldState.brushPos = operation.brushPosition;
        worldState.isBrushHighlightVisible = operation.mode != InteractionMode::MoveCamera;
        worldState.brushRadius = state.brushRadius / 2048.0f;
        worldState.brushFalloff = state.brushFalloff;

        // update material properties
        for (uint32 i = 0; i < MATERIAL_COUNT; i++)
        {
            const MaterialProperties *stateProps = &state.materialProps[i];
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
        lightDir.x = sin(state.lightDirection * glm::pi<float>() * -0.5);
        lightDir.y = cos(state.lightDirection * glm::pi<float>() * 0.5);
        lightDir.z = 0.2f;
        rendererUpdateLightingState(ctx.memory, &lightDir, true, true, true, true, true);
    }

    bool SceneWorld::updateViewState(ViewState *viewState, float deltaTime)
    {
        bool isManipulatingCamera = false;
        const IO::MouseInputState &mouseState =
            ctx.input.getMouseState(viewState->inputControllerId);

        // orbit distance is modified by scrolling the mouse wheel
        viewState->orbitCameraDistance *= 1.0f - (glm::sign(mouseState.scrollOffsetY) * 0.05f);

        if (ctx.input.isMouseButtonDown(viewState->inputControllerId, IO::MouseButton::Middle))
        {
            // update the look at position if the middle mouse button is pressed
            glm::vec3 lookDir = glm::normalize(viewState->cameraLookAt - viewState->cameraPos);
            glm::vec3 xDir = cross(lookDir, glm::vec3(0, -1, 0));
            glm::vec3 yDir = cross(lookDir, xDir);
            glm::vec3 pan =
                (xDir * mouseState.cursorOffsetX) + (yDir * mouseState.cursorOffsetY);
            float panMagnitude = glm::clamp(viewState->orbitCameraDistance, 2.5f, 300.0f);
            viewState->cameraLookAt += pan * panMagnitude * 0.02f * deltaTime;

            isManipulatingCamera = true;
        }
        if (ctx.input.isMouseButtonDown(viewState->inputControllerId, IO::MouseButton::Right))
        {
            // update yaw & pitch if the right mouse button is pressed
            float rotateMagnitude = glm::clamp(viewState->orbitCameraDistance, 14.0f, 70.0f);
            float rotateSensitivity = 0.05f * rotateMagnitude * deltaTime;
            viewState->orbitCameraYaw +=
                glm::radians(mouseState.cursorOffsetX * rotateSensitivity);
            viewState->orbitCameraPitch +=
                glm::radians(mouseState.cursorOffsetY * rotateSensitivity);

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

    SceneWorld::OperationState SceneWorld::getCurrentOperation(const EditorState &prevState)
    {
        EditorTool tool = prevState.tool;

        // first pass - loop through each camera and determine whether the camera is being
        // moved or the active brush stroke is being discarded
        for (int i = 0; i < viewStateCount; i++)
        {
            ViewState *viewState = &viewStates[i];
            if (ctx.input.isMouseButtonDown(
                    viewState->inputControllerId, IO::MouseButton::Middle)
                || ctx.input.isMouseButtonDown(
                    viewState->inputControllerId, IO::MouseButton::Right))
            {
                return {
                    InteractionMode::MoveCamera, // mode
                    tool,                        // tool
                    false,                       // isBrushActive
                    false,                       // isDiscardingStroke
                    glm::vec2(),                 // brushPosition
                    0.0f,                        // brushRadiusIncrease
                    0.0f                         // brushFalloffIncrease
                };
            }
            if (prevState.heightmapStatus == HeightmapStatus::Editing
                && ctx.input.isKeyDown(viewState->inputControllerId, IO::Key::Escape))
            {
                return {
                    InteractionMode::PaintBrushStroke, // mode
                    tool,                              // tool
                    false,                             // isBrushActive
                    true,                              // isDiscardingStroke
                    glm::vec2(),                       // brushPosition
                    0.0f,                              // brushRadiusIncrease
                    0.0f                               // brushFalloffIncrease
                };
            }
        }

        // second pass - loop through each camera and determine whether the brush is active
        for (int i = 0; i < viewStateCount; i++)
        {
            ViewState *viewState = &viewStates[i];
            const IO::MouseInputState &mouseState =
                ctx.input.getMouseState(viewState->inputControllerId);
            float mouseX = (mouseState.normalizedCursorX * 2.0f) - 1.0f;
            float mouseY = (mouseState.normalizedCursorY * 2.0f) - 1.0f;

            glm::mat4 inverseViewProjection = glm::inverse(viewState->cameraTransform);
            glm::vec4 screenPos = glm::vec4(mouseX, -mouseY, 1.0f, 1.0f);
            glm::vec4 worldPos = inverseViewProjection * screenPos;

            glm::vec3 intersectionPoint;
            if (!heightfieldIsRayIntersecting(&heightfield, viewState->cameraPos,
                    glm::normalize(glm::vec3(worldPos)), intersectionPoint))
                continue;

            glm::vec2 normalizedPickPoint = glm::vec2(
                (intersectionPoint.x / 127.5f) + 0.5f, (intersectionPoint.z / 127.5f) + 0.5f);

            // if the R key is pressed, we are adjusting the brush radius
            if (ctx.input.isKeyDown(viewState->inputControllerId, IO::Key::R))
            {
                float brushRadiusIncrease =
                    mouseState.cursorOffsetX + mouseState.cursorOffsetY;

                return {
                    InteractionMode::ModifyBrushRadius, // mode
                    tool,                               // tool
                    false,                              // isBrushActive
                    false,                              // isDiscardingStroke
                    normalizedPickPoint,                // brushPosition
                    brushRadiusIncrease,                // brushRadiusIncrease
                    0.0f                                // brushFalloffIncrease
                };
            }

            // if the F key is pressed, we are adjusting the brush falloff
            if (ctx.input.isKeyDown(viewState->inputControllerId, IO::Key::F))
            {
                float brushFalloffIncrease =
                    (mouseState.cursorOffsetX + mouseState.cursorOffsetY) * 0.001f;

                return {
                    InteractionMode::ModifyBrushFalloff, // mode
                    tool,                                // tool
                    false,                               // isBrushActive
                    false,                               // isDiscardingStroke
                    normalizedPickPoint,                 // brushPosition
                    0.0f,                                // brushRadiusIncrease
                    brushFalloffIncrease                 // brushFalloffIncrease
                };
            }

            // if a number key is pressed, change the selected tool
            if (ctx.input.isKeyDown(viewState->inputControllerId, IO::Key::D1))
            {
                tool = EditorTool::RaiseTerrain;
            }
            else if (ctx.input.isKeyDown(viewState->inputControllerId, IO::Key::D2))
            {
                tool = EditorTool::LowerTerrain;
            }

            // the LMB must be newly pressed to start a new brush stroke
            bool isBrushActive = false;
            if (prevState.heightmapStatus == HeightmapStatus::Editing
                && ctx.input.isMouseButtonDown(
                    viewState->inputControllerId, IO::MouseButton::Left))
            {
                isBrushActive = true;
            }
            else if (ctx.input.isNewMouseButtonPress(
                         viewState->inputControllerId, IO::MouseButton::Left))
            {
                isBrushActive = true;
            }

            return {
                InteractionMode::PaintBrushStroke, // mode
                tool,                              // tool
                isBrushActive,                     // isBrushActive
                false,                             // isDiscardingStroke
                normalizedPickPoint,               // brushPosition
                0.0f,                              // brushRadiusIncrease
                0.0f                               // brushFalloffIncrease
            };
        }

        return {
            InteractionMode::PaintBrushStroke, // mode
            tool,                              // tool
            false,                             // isBrushActive
            false,                             // isDiscardingStroke
            glm::vec2(),                       // brushPosition
            0.0f,                              // brushRadiusIncrease
            0.0f                               // brushFalloffIncrease
        };
    }

    HeightmapStatus SceneWorld::getNextHeightmapStatus(
        HeightmapStatus currentHeightmapStatus, bool isBrushActive, bool isDiscardingStroke)
    {
        switch (currentHeightmapStatus)
        {
        case HeightmapStatus::Committing:
        case HeightmapStatus::Idle:
            return isBrushActive ? HeightmapStatus::Editing : HeightmapStatus::Idle;
        case HeightmapStatus::Editing:
            return isDiscardingStroke
                ? HeightmapStatus::Discarding
                : (isBrushActive ? HeightmapStatus::Editing : HeightmapStatus::Committing);
        case HeightmapStatus::Discarding:
            return HeightmapStatus::Committing;
        }

        return currentHeightmapStatus;
    }

    void SceneWorld::render(
        EngineMemory *memory, uint32 viewportWidth, uint32 viewportHeight, int32 viewId)
    {
        if (viewportWidth == 0 || viewportHeight == 0 || viewId == -1)
            return;

        assert(viewId < MAX_SCENE_VIEWS);
        ViewState *viewState = &viewStates[viewId];

        // calculate camera transform
        constexpr float fov = glm::pi<float>() / 4.0f;
        const float nearPlane = 0.1f;
        const float farPlane = 10000.0f;
        const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        const float aspectRatio = (float)viewportWidth / (float)viewportHeight;
        glm::mat4 projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
        viewState->cameraTransform =
            projection * glm::lookAt(viewState->cameraPos, viewState->cameraLookAt, up);

        rendererUpdateCameraState(memory, &viewState->cameraTransform);
        rendererSetViewportSize(viewportWidth, viewportHeight);
        rendererClearBackBuffer(0.392f, 0.584f, 0.929f, 1);

        // get textures
        for (uint32 layerIdx = 0; layerIdx < MATERIAL_COUNT; layerIdx++)
        {
            uint32 assetId;
            TextureAsset *asset;
            TextureAssetBinding *binding;

            assetId = worldState.albedoTextureAssetIds[layerIdx];
            if (assetId)
            {
                binding = &albedoTextures[layerIdx];
                asset = assetsGetTexture(memory, assetId);
                if (asset
                    && (assetId != binding->assetId || asset->version > binding->version))
                {
                    rendererUpdateTextureArray(memory, albedoTextureArrayHandle,
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
                asset = assetsGetTexture(memory, assetId);
                if (asset
                    && (assetId != binding->assetId || asset->version > binding->version))
                {
                    rendererUpdateTextureArray(memory, normalTextureArrayHandle,
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
                asset = assetsGetTexture(memory, assetId);
                if (asset
                    && (assetId != binding->assetId || asset->version > binding->version))
                {
                    rendererUpdateTextureArray(memory, displacementTextureArrayHandle,
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
                asset = assetsGetTexture(memory, assetId);
                if (asset
                    && (assetId != binding->assetId || asset->version > binding->version))
                {
                    rendererUpdateTextureArray(memory, aoTextureArrayHandle, GL_UNSIGNED_BYTE,
                        GL_RED, asset->width, asset->height, layerIdx, asset->data);
                    binding->assetId = assetId;
                    binding->version = asset->version;
                }
            }
        }

        // get shader programs
        ShaderProgramAsset *calcTessLevelShaderProgramAsset =
            assetsGetShaderProgram(memory, ASSET_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL);
        ShaderProgramAsset *terrainShaderProgramAsset =
            assetsGetShaderProgram(memory, ASSET_SHADER_PROGRAM_TERRAIN_TEXTURED);
        if (!calcTessLevelShaderProgramAsset || !terrainShaderProgramAsset)
            return;

        // calculate tessellation levels
        uint32 calcTessLevelShaderProgramHandle = calcTessLevelShaderProgramAsset->handle;
        rendererSetShaderProgramUniformFloat(
            memory, calcTessLevelShaderProgramHandle, "targetTriangleSize", 0.015f);

        constexpr int terrainRows = 256;
        constexpr int terrainColumns = 256;
        constexpr float patchSize = 0.5f;
        constexpr float terrainHeight = 25.0f;

        rendererSetShaderProgramUniformInteger(memory, calcTessLevelShaderProgramHandle,
            "horizontalEdgeCount", terrainRows * (terrainColumns - 1));
        rendererSetShaderProgramUniformInteger(
            memory, calcTessLevelShaderProgramHandle, "columnCount", terrainColumns);
        rendererSetShaderProgramUniformFloat(
            memory, calcTessLevelShaderProgramHandle, "terrainHeight", terrainHeight);
        rendererBindTexture(memory, heightmapTextureHandle, 0);

        int meshEdgeCount =
            (2 * (terrainRows * terrainColumns)) - terrainRows - terrainColumns;
        uint32 meshVertexBufferHandle =
            ctx.assets.graphics.getMeshVertexBufferHandle(meshHandle, 0);

        rendererBindShaderStorageBuffer(memory, tessellationLevelBufferHandle, 0);
        rendererBindShaderStorageBuffer(memory, meshVertexBufferHandle, 1);
        rendererUseShaderProgram(memory, calcTessLevelShaderProgramHandle);
        rendererDispatchCompute(meshEdgeCount, 1, 1);
        rendererShaderStorageMemoryBarrier();

        // bind material data
        rendererUpdateBuffer(ctx.memory, materialPropsBufferHandle,
            sizeof(worldState.materialProps), worldState.materialProps);

        uint32 terrainShaderProgramHandle = terrainShaderProgramAsset->handle;
        rendererUseShaderProgram(memory, terrainShaderProgramHandle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        rendererBindTexture(memory, heightmapTextureHandle, 0);
        rendererBindTextureArray(memory, albedoTextureArrayHandle, 1);
        rendererBindTextureArray(memory, normalTextureArrayHandle, 2);
        rendererBindTextureArray(memory, displacementTextureArrayHandle, 3);
        rendererBindTextureArray(memory, aoTextureArrayHandle, 4);
        rendererBindShaderStorageBuffer(memory, materialPropsBufferHandle, 1);

        // bind mesh data
        int elementCount = ctx.assets.graphics.getMeshElementCount(meshHandle);
        unsigned int primitiveType = ctx.assets.graphics.getMeshPrimitiveType(meshHandle);
        rendererBindVertexArray(
            memory, ctx.assets.graphics.getMeshVertexArrayHandle(meshHandle));

        // set shader uniforms
        rendererSetShaderProgramUniformVector3(memory, terrainShaderProgramHandle,
            "terrainDimensions",
            glm::vec3(patchSize * terrainColumns, terrainHeight, patchSize * terrainRows));
        rendererSetShaderProgramUniformFloat(memory, terrainShaderProgramHandle,
            "brushHighlightStrength", worldState.isBrushHighlightVisible ? 0.4f : 0.0f);
        rendererSetShaderProgramUniformVector2(
            memory, terrainShaderProgramHandle, "brushHighlightPos", worldState.brushPos);
        rendererSetShaderProgramUniformFloat(memory, terrainShaderProgramHandle,
            "brushHighlightRadius", worldState.brushRadius);
        rendererSetShaderProgramUniformFloat(memory, terrainShaderProgramHandle,
            "brushHighlightFalloff", worldState.brushFalloff);

        // draw mesh
        rendererDrawElementsInstanced(primitiveType, elementCount, 1);
    }
}}}}