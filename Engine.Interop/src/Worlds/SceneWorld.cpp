#include "SceneWorld.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    SceneWorld::SceneWorld(EngineContext &ctx) : ctx(ctx), world(ctx)
    {
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
        std::vector<std::string> materialUniformNames(5);
        materialUniformNames[0] = "terrainHeight";
        materialUniformNames[1] = "heightmapSize";
        materialUniformNames[2] = "normalSampleOffset";
        materialUniformNames[3] = "brushHighlightStrength";
        materialUniformNames[4] = "brushHighlightPos";

        std::vector<Terrain::Engine::Graphics::UniformValue> materialUniformValues(5);
        materialUniformValues[0] =
            Terrain::Engine::Graphics::UniformValue::forFloat(terrainHeight);
        materialUniformValues[1] =
            Terrain::Engine::Graphics::UniformValue::forVector2(glm::vec2(1.0f, 1.0f));
        materialUniformValues[2] = Terrain::Engine::Graphics::UniformValue::forVector2(
            glm::vec2(1.0f / (patchSize * terrainColumns), 1.0f / (patchSize * terrainRows)));
        materialUniformValues[3] = Terrain::Engine::Graphics::UniformValue::forFloat(0.4f);
        materialUniformValues[4] =
            Terrain::Engine::Graphics::UniformValue::forVector2(glm::vec2(0.5f, 0.5f));

        // create entity and components
        int entityId = ctx.entities.create();
        int terrainRendererInstanceId =
            world.componentManagers.terrainRenderer.create(entityId, -1,
                heightmapTextureHandle, terrainRows, terrainColumns, patchSize, terrainHeight);

        int &meshHandle =
            world.componentManagers.terrainRenderer.getMeshHandle(terrainRendererInstanceId);
        terrainMeshRendererInstanceId = world.componentManagers.meshRenderer.create(
            entityId, meshHandle, materialHandle, materialUniformNames, materialUniformValues);

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
        if (state.wasHeightmapUpdated)
        {
            // update terrain collider with composited heightmap texture
            void *textureData = malloc(2048 * 2048 * 2);
            ctx.renderer.getTexturePixels(heightmapTextureHandle, textureData);
            world.componentManagers.terrainCollider.updateHeights(terrainColliderInstanceId,
                2048, 2048, static_cast<const unsigned short *>(textureData));
            free(textureData);
        }

        world.update(deltaTime);

        int cameraCount = orbitCameraIds.size();
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

            IO::MouseInputState &mouseState = ctx.input.getMouseState(inputControllerId);

            if (mouseState.isMiddleMouseButtonDown || mouseState.isRightMouseButtonDown)
                continue;

            glm::vec2 normalizedPickPoint = glm::vec2(
                (intersectionPoint.x / 127.5f) + 0.5f, (intersectionPoint.z / 127.5f) + 0.5f);
            world.componentManagers.meshRenderer.setMaterialUniformVector2(
                terrainMeshRendererInstanceId, "brushHighlightPos", normalizedPickPoint);

            if (mouseState.isLeftMouseButtonDown)
            {
                newState.brushQuadX = normalizedPickPoint.x;
                newState.brushQuadY = normalizedPickPoint.y;
                newState.doesHeightmapRequireRedraw = true;
            }
        }
    }

    void SceneWorld::render(EngineViewContext &vctx)
    {
        world.render(vctx);
    }
}}}}