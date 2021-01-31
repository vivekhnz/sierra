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

        // create entity and components
        int entityId = ctx.entities.create();
        int terrainRendererInstanceId =
            world.componentManagers.terrainRenderer.create(entityId, -1,
                heightmapTextureHandle, terrainRows, terrainColumns, patchSize, terrainHeight);

        meshHandle =
            world.componentManagers.terrainRenderer.getMeshHandle(terrainRendererInstanceId);
        world.componentManagers.meshRenderer.create(entityId, meshHandle, 0, 0, 0, 0, 1);

        terrainColliderInstanceId = world.componentManagers.terrainCollider.create(
            entityId, -1, terrainRows, terrainColumns, patchSize, terrainHeight);

        // create buffer to store vertex edge data
        tessellationLevelBufferHandle =
            rendererCreateBuffer(ctx.memory, RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
        rendererUpdateBuffer(ctx.memory, tessellationLevelBufferHandle,
            terrainColumns * terrainRows * 10 * sizeof(glm::vec4), 0);
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
        worldState.brushPos = operation.brushPosition;
        worldState.isBrushHighlightVisible = operation.mode != InteractionMode::MoveCamera;
        worldState.brushRadius = state.brushRadius / 2048.0f;
        worldState.brushFalloff = state.brushFalloff;

        // update material properties
        worldState.mat1_textureSizeInWorldUnits =
            glm::vec2(state.mat1_textureSize, state.mat1_textureSize);
        worldState.mat2_textureSizeInWorldUnits =
            glm::vec2(state.mat2_textureSize, state.mat2_textureSize);
        worldState.mat2_rampParams =
            glm::vec4(state.mat2_rampParams.slopeStart, state.mat2_rampParams.slopeEnd,
                state.mat2_rampParams.altitudeStart, state.mat2_rampParams.altitudeEnd);
        worldState.mat3_textureSizeInWorldUnits =
            glm::vec2(state.mat3_textureSize, state.mat3_textureSize);
        worldState.mat3_rampParams =
            glm::vec4(state.mat3_rampParams.slopeStart, state.mat3_rampParams.slopeEnd,
                state.mat3_rampParams.altitudeStart, state.mat3_rampParams.altitudeEnd);

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
        constexpr int heightmapWidth = 2048;
        constexpr int heightmapHeight = 2048;

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
        uint32 terrainShaderProgramHandle = terrainShaderProgramAsset->handle;
        rendererUseShaderProgram(memory, terrainShaderProgramHandle);
        rendererSetPolygonMode(GL_FILL);
        rendererSetBlendMode(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const int RESOURCE_ID_TEXTURE_GROUND_ALBEDO = 1;
        const int RESOURCE_ID_TEXTURE_GROUND_NORMAL = 2;
        const int RESOURCE_ID_TEXTURE_GROUND_DISPLACEMENT = 3;
        const int RESOURCE_ID_TEXTURE_GROUND_AO = 4;
        const int RESOURCE_ID_TEXTURE_ROCK_ALBEDO = 5;
        const int RESOURCE_ID_TEXTURE_ROCK_NORMAL = 6;
        const int RESOURCE_ID_TEXTURE_ROCK_DISPLACEMENT = 7;
        const int RESOURCE_ID_TEXTURE_ROCK_AO = 8;
        const int RESOURCE_ID_TEXTURE_SNOW_ALBEDO = 9;
        const int RESOURCE_ID_TEXTURE_SNOW_NORMAL = 10;
        const int RESOURCE_ID_TEXTURE_SNOW_DISPLACEMENT = 11;
        const int RESOURCE_ID_TEXTURE_SNOW_AO = 12;

        rendererBindTexture(memory, heightmapTextureHandle, 0);
        rendererBindTexture(
            memory, ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_GROUND_ALBEDO), 1);
        rendererBindTexture(
            memory, ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_GROUND_NORMAL), 2);
        rendererBindTexture(
            memory, ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_GROUND_DISPLACEMENT), 3);
        rendererBindTexture(
            memory, ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_GROUND_AO), 4);
        rendererBindTexture(
            memory, ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_ROCK_ALBEDO), 5);
        rendererBindTexture(
            memory, ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_ROCK_NORMAL), 6);
        rendererBindTexture(
            memory, ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_ROCK_DISPLACEMENT), 7);
        rendererBindTexture(
            memory, ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_ROCK_AO), 8);
        rendererBindTexture(
            memory, ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_SNOW_ALBEDO), 9);
        rendererBindTexture(
            memory, ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_SNOW_NORMAL), 10);
        rendererBindTexture(
            memory, ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_SNOW_DISPLACEMENT), 11);
        rendererBindTexture(
            memory, ctx.renderer.lookupTexture(RESOURCE_ID_TEXTURE_SNOW_AO), 12);

        // bind mesh data
        int elementCount = ctx.assets.graphics.getMeshElementCount(meshHandle);
        unsigned int primitiveType = ctx.assets.graphics.getMeshPrimitiveType(meshHandle);
        rendererBindVertexArray(
            memory, ctx.assets.graphics.getMeshVertexArrayHandle(meshHandle));

        // set shader uniforms
        rendererSetShaderProgramUniformVector2(memory, terrainShaderProgramHandle,
            "heightmapSize", glm::vec2(heightmapWidth, heightmapHeight));
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
        rendererSetShaderProgramUniformVector2(memory, terrainShaderProgramHandle,
            "mat1_textureSizeInWorldUnits", worldState.mat1_textureSizeInWorldUnits);
        rendererSetShaderProgramUniformVector2(memory, terrainShaderProgramHandle,
            "mat2_textureSizeInWorldUnits", worldState.mat2_textureSizeInWorldUnits);
        rendererSetShaderProgramUniformVector4(
            memory, terrainShaderProgramHandle, "mat2_rampParams", worldState.mat2_rampParams);
        rendererSetShaderProgramUniformVector2(memory, terrainShaderProgramHandle,
            "mat3_textureSizeInWorldUnits", worldState.mat3_textureSizeInWorldUnits);
        rendererSetShaderProgramUniformVector4(
            memory, terrainShaderProgramHandle, "mat3_rampParams", worldState.mat3_rampParams);

        // draw mesh
        rendererDrawElementsInstanced(primitiveType, elementCount, 1);
    }
}}}}