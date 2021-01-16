#include "SceneWorld.hpp"
#include <glm/gtc/type_ptr.hpp>

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
        std::vector<std::string> materialUniformNames(11);
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

        std::vector<Terrain::Engine::Graphics::UniformValue> materialUniformValues(11);
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
            meshHandle, materialHandle, materialUniformNames, materialUniformValues, 1);

        terrainColliderInstanceId = world.componentManagers.terrainCollider.create(
            entityId, -1, terrainRows, terrainColumns, patchSize, terrainHeight);
    }

    void SceneWorld::linkViewport(ViewportContext &vctx)
    {
        int cameraEntityId = ctx.entities.create();
        world.componentManagers.camera.create(
            cameraEntityId, glm::vec4(0.392f, 0.584f, 0.929f, 1.0f), -1);

        int orbitCameraId = world.componentManagers.orbitCamera.create(cameraEntityId);
        world.componentManagers.orbitCamera.setPitch(orbitCameraId, glm::radians(15.0f));
        world.componentManagers.orbitCamera.setYaw(orbitCameraId, glm::radians(180.0f));
        world.componentManagers.orbitCamera.setDistance(orbitCameraId, 112.5f);
        world.componentManagers.orbitCamera.setInputControllerId(
            orbitCameraId, vctx.getInputControllerId());

        vctx.setCameraEntityId(cameraEntityId);
        orbitCameraIds.push_back(orbitCameraId);
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

        world.update(deltaTime);

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

        Terrain::Engine::Graphics::Renderer::LightingState lighting = {
            lightDir, // lightDir
            1,        // isEnabled
            1,        // isTextureEnabled
            1,        // isNormalMapEnabled
            1,        // isAOMapEnabled
            1         // isDisplacementMapEnabled
        };
        ctx.renderer.updateUniformBuffer(
            Terrain::Engine::Graphics::Renderer::UniformBuffer::Lighting, &lighting);
    }

    SceneWorld::OperationState SceneWorld::getCurrentOperation(const EditorState &prevState)
    {
        EditorTool tool = prevState.tool;

        // first pass - loop through each camera and determine whether the camera is being
        // moved or the active brush stroke is being discarded
        int cameraCount = orbitCameraIds.size();
        for (int i = 0; i < cameraCount; i++)
        {
            int orbitCameraId = orbitCameraIds[i];
            int inputControllerId =
                world.componentManagers.orbitCamera.getInputControllerId(orbitCameraId);

            if (ctx.input.isMouseButtonDown(inputControllerId, IO::MouseButton::Middle)
                || ctx.input.isMouseButtonDown(inputControllerId, IO::MouseButton::Right))
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
                && ctx.input.isKeyDown(inputControllerId, IO::Key::Escape))
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
        for (int i = 0; i < cameraCount; i++)
        {
            int orbitCameraId = orbitCameraIds[i];
            int inputControllerId =
                world.componentManagers.orbitCamera.getInputControllerId(orbitCameraId);

            Physics::Ray ray = world.componentManagers.orbitCamera.getPickRay(orbitCameraId);

            glm::vec3 intersectionPoint;
            if (!world.componentManagers.terrainCollider.intersects(
                    terrainColliderInstanceId, ray, intersectionPoint))
                continue;

            glm::vec2 normalizedPickPoint = glm::vec2(
                (intersectionPoint.x / 127.5f) + 0.5f, (intersectionPoint.z / 127.5f) + 0.5f);
            world.componentManagers.meshRenderer.setMaterialUniformVector2(
                terrainMeshRendererInstanceId, "brushHighlightPos", normalizedPickPoint);

            IO::MouseInputState mouseState = ctx.input.getMouseState(inputControllerId);

            // if the R key is pressed, we are adjusting the brush radius
            if (ctx.input.isKeyDown(inputControllerId, IO::Key::R))
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
            if (ctx.input.isKeyDown(inputControllerId, IO::Key::F))
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
            if (ctx.input.isKeyDown(inputControllerId, IO::Key::D1))
            {
                tool = EditorTool::RaiseTerrain;
            }
            else if (ctx.input.isKeyDown(inputControllerId, IO::Key::D2))
            {
                tool = EditorTool::LowerTerrain;
            }

            // the LMB must be newly pressed to start a new brush stroke
            bool isBrushActive =
                (prevState.heightmapStatus == HeightmapStatus::Editing
                    && ctx.input.isMouseButtonDown(inputControllerId, IO::MouseButton::Left))
                || ctx.input.isNewMouseButtonPress(inputControllerId, IO::MouseButton::Left);

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

    void SceneWorld::render(EngineViewContext &vctx)
    {
        world.render(vctx);
    }
}}}}