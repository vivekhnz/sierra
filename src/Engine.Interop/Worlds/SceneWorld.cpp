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

    void SceneWorld::initialize(int heightmapTextureHandle)
    {
        int terrainColumns = 256;
        int terrainRows = 256;
        float patchSize = 0.5f;
        float terrainHeight = 25.0f;

        this->heightmapTextureHandle = heightmapTextureHandle;

        const int RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED = 0;

        // edit terrain material to point to composited heightmap texture handle
        int &materialHandle =
            ctx.assets.graphics.lookupMaterial(RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED);
        ctx.assets.graphics.setMaterialTexture(materialHandle, 0, heightmapTextureHandle);

        // build material uniforms
        const char *materialUniformNames[11];
        materialUniformNames[0] = "heightmapSize";
        materialUniformNames[1] = "terrainDimensions";
        materialUniformNames[2] = "brushHighlightStrength";
        materialUniformNames[3] = "brushHighlightPos";
        materialUniformNames[4] = "brushHighlightRadius";
        materialUniformNames[5] = "brushHighlightFalloff";
        materialUniformNames[6] = "mat1_textureSizeInWorldUnits";
        materialUniformNames[7] = "mat2_textureSizeInWorldUnits";
        materialUniformNames[8] = "mat2_rampParams";
        materialUniformNames[9] = "mat3_textureSizeInWorldUnits";
        materialUniformNames[10] = "mat3_rampParams";

        Graphics::UniformValue materialUniformValues[11];
        materialUniformValues[0] =
            Terrain::Engine::Graphics::UniformValue::forVector2(glm::vec2(1.0f, 1.0f));
        materialUniformValues[1] = Terrain::Engine::Graphics::UniformValue::forVector3(
            glm::vec3(patchSize * terrainColumns, terrainHeight, patchSize * terrainRows));
        materialUniformValues[2] = Terrain::Engine::Graphics::UniformValue::forFloat(0.4f);
        materialUniformValues[3] =
            Terrain::Engine::Graphics::UniformValue::forVector2(glm::vec2(0.5f, 0.5f));
        materialUniformValues[4] =
            Terrain::Engine::Graphics::UniformValue::forFloat(128 / 2048.0f);
        materialUniformValues[5] = Terrain::Engine::Graphics::UniformValue::forFloat(0.75f);
        materialUniformValues[6] =
            Terrain::Engine::Graphics::UniformValue::forVector2(glm::vec2(2.5f, 2.5f));
        materialUniformValues[7] =
            Terrain::Engine::Graphics::UniformValue::forVector2(glm::vec2(13.0f, 13.0f));
        materialUniformValues[8] = Terrain::Engine::Graphics::UniformValue::forVector4(
            glm::vec4(0.6f, 0.8f, 0, 0.001f));
        materialUniformValues[9] =
            Terrain::Engine::Graphics::UniformValue::forVector2(glm::vec2(2.0f, 2.0f));
        materialUniformValues[10] = Terrain::Engine::Graphics::UniformValue::forVector4(
            glm::vec4(0.8f, 0.75f, 0.25f, 0.28f));

        // create entity and components
        int entityId = ctx.entities.create();
        int terrainRendererInstanceId =
            world.componentManagers.terrainRenderer.create(entityId, -1,
                heightmapTextureHandle, terrainRows, terrainColumns, patchSize, terrainHeight);

        int &meshHandle =
            world.componentManagers.terrainRenderer.getMeshHandle(terrainRendererInstanceId);
        terrainMeshRendererInstanceId = world.componentManagers.meshRenderer.create(entityId,
            meshHandle, materialHandle, 11, materialUniformNames, materialUniformValues, 1);

        terrainColliderInstanceId = world.componentManagers.terrainCollider.create(
            entityId, -1, terrainRows, terrainColumns, patchSize, terrainHeight);

        // create buffer to store vertex edge data
        tessellationLevelBufferHandle =
            rendererCreateBuffer(ctx.memory, RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
        rendererUpdateBuffer(ctx.memory, tessellationLevelBufferHandle,
            terrainColumns * terrainRows * 10 * sizeof(glm::vec4), 0);

        meshVertexBufferHandle = ctx.assets.graphics.getMeshVertexBufferHandle(meshHandle, 0);
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
            // update terrain collider with composited heightmap texture
            ctx.renderer.getTexturePixels(
                heightmapTextureHandle, heightmapTextureDataTempBuffer);
            world.componentManagers.terrainCollider.updateHeights(terrainColliderInstanceId,
                2048, 2048,
                static_cast<const unsigned short *>(heightmapTextureDataTempBuffer));
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
        bool isBrushHiglightVisible = operation.mode != InteractionMode::MoveCamera;
        world.componentManagers.meshRenderer.setMaterialUniformVector2(
            terrainMeshRendererInstanceId, "brushHighlightPos", operation.brushPosition);
        world.componentManagers.meshRenderer.setMaterialUniformFloat(
            terrainMeshRendererInstanceId, "brushHighlightStrength",
            isBrushHiglightVisible ? 0.4f : 0.0f);
        world.componentManagers.meshRenderer.setMaterialUniformFloat(
            terrainMeshRendererInstanceId, "brushHighlightRadius",
            state.brushRadius / 2048.0f);
        world.componentManagers.meshRenderer.setMaterialUniformFloat(
            terrainMeshRendererInstanceId, "brushHighlightFalloff", state.brushFalloff);

        // update material properties
        world.componentManagers.meshRenderer.setMaterialUniformVector2(
            terrainMeshRendererInstanceId, "mat1_textureSizeInWorldUnits",
            glm::vec2(state.mat1_textureSize, state.mat1_textureSize));
        world.componentManagers.meshRenderer.setMaterialUniformVector2(
            terrainMeshRendererInstanceId, "mat2_textureSizeInWorldUnits",
            glm::vec2(state.mat2_textureSize, state.mat2_textureSize));
        world.componentManagers.meshRenderer.setMaterialUniformVector4(
            terrainMeshRendererInstanceId, "mat2_rampParams",
            glm::vec4(state.mat2_rampParams.slopeStart, state.mat2_rampParams.slopeEnd,
                state.mat2_rampParams.altitudeStart, state.mat2_rampParams.altitudeEnd));
        world.componentManagers.meshRenderer.setMaterialUniformVector2(
            terrainMeshRendererInstanceId, "mat3_textureSizeInWorldUnits",
            glm::vec2(state.mat3_textureSize, state.mat3_textureSize));
        world.componentManagers.meshRenderer.setMaterialUniformVector4(
            terrainMeshRendererInstanceId, "mat3_rampParams",
            glm::vec4(state.mat3_rampParams.slopeStart, state.mat3_rampParams.slopeEnd,
                state.mat3_rampParams.altitudeStart, state.mat3_rampParams.altitudeEnd));

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
            float panMagnitude =
                std::min(std::max(viewState->orbitCameraDistance, 2.5f), 300.0f);
            viewState->cameraLookAt += pan * panMagnitude * 0.02f * deltaTime;

            isManipulatingCamera = true;
        }
        if (ctx.input.isMouseButtonDown(viewState->inputControllerId, IO::MouseButton::Right))
        {
            // update yaw & pitch if the right mouse button is pressed
            float rotateMagnitude =
                std::min(std::max(viewState->orbitCameraDistance, 14.0f), 70.0f);
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

            Physics::Ray ray = {};
            ray.origin = viewState->cameraPos;
            ray.direction = glm::normalize(glm::vec3(worldPos));

            glm::vec3 intersectionPoint;
            if (!world.componentManagers.terrainCollider.intersects(
                    terrainColliderInstanceId, ray, intersectionPoint))
                continue;

            glm::vec2 normalizedPickPoint = glm::vec2(
                (intersectionPoint.x / 127.5f) + 0.5f, (intersectionPoint.z / 127.5f) + 0.5f);
            world.componentManagers.meshRenderer.setMaterialUniformVector2(
                terrainMeshRendererInstanceId, "brushHighlightPos", normalizedPickPoint);

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

        // calculate tessellation levels
        ShaderProgramAsset *calcTessLevelShaderProgramAsset =
            assetsGetShaderProgram(memory, ASSET_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL);
        if (!calcTessLevelShaderProgramAsset)
            return;

        uint32 calcTessLevelShaderProgramHandle = calcTessLevelShaderProgramAsset->handle;
        rendererSetShaderProgramUniformFloat(
            memory, calcTessLevelShaderProgramHandle, "targetTriangleSize", 0.015f);

        constexpr int rows = 256;
        constexpr int columns = 256;
        constexpr float terrainHeight = 25.0f;

        rendererSetShaderProgramUniformInteger(memory, calcTessLevelShaderProgramHandle,
            "horizontalEdgeCount", rows * (columns - 1));
        rendererSetShaderProgramUniformInteger(
            memory, calcTessLevelShaderProgramHandle, "columnCount", columns);
        rendererSetShaderProgramUniformFloat(
            memory, calcTessLevelShaderProgramHandle, "terrainHeight", terrainHeight);
        rendererBindTexture(memory, heightmapTextureHandle, 0);

        int meshEdgeCount = (2 * (rows * columns)) - rows - columns;
        rendererBindShaderStorageBuffer(memory, tessellationLevelBufferHandle, 0);
        rendererBindShaderStorageBuffer(memory, meshVertexBufferHandle, 1);
        rendererUseShaderProgram(memory, calcTessLevelShaderProgramHandle);
        rendererDispatchCompute(meshEdgeCount, 1, 1);
        rendererShaderStorageMemoryBarrier();

        // render terrain
        world.componentManagers.meshRenderer.renderMeshes();
    }
}}}}