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

        const int RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED = 0;

        // edit terrain material to point to composited heightmap texture handle
        int &materialHandle =
            ctx.assets.graphics.lookupMaterial(RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED);
        ctx.assets.graphics.setMaterialTexture(materialHandle, 0, heightmapTextureHandle);

        // build material uniforms
        std::vector<std::string> materialUniformNames(3);
        materialUniformNames[0] = "terrainHeight";
        materialUniformNames[1] = "heightmapSize";
        materialUniformNames[2] = "normalSampleOffset";

        std::vector<Terrain::Engine::Graphics::UniformValue> materialUniformValues(3);
        materialUniformValues[0] =
            Terrain::Engine::Graphics::UniformValue::forFloat(terrainHeight);
        materialUniformValues[1] =
            Terrain::Engine::Graphics::UniformValue::forVector2(glm::vec2(1.0f, 1.0f));
        materialUniformValues[2] = Terrain::Engine::Graphics::UniformValue::forVector2(
            glm::vec2(1.0f / (patchSize * terrainColumns), 1.0f / (patchSize * terrainRows)));

        // create entity and components
        int entityId = ctx.entities.create();
        int terrainRendererInstanceId =
            world.componentManagers.terrainRenderer.create(entityId, -1,
                heightmapTextureHandle, terrainRows, terrainColumns, patchSize, terrainHeight);

        int &meshHandle =
            world.componentManagers.terrainRenderer.getMeshHandle(terrainRendererInstanceId);
        world.componentManagers.meshRenderer.create(
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
        world.update(deltaTime);

        int cameraCount = orbitCameraIds.size();
        for (int i = 0; i < cameraCount; i++)
        {
            int orbitCameraId = orbitCameraIds[i];
            int inputControllerId =
                world.componentManagers.orbitCamera.getInputControllerId(orbitCameraId);

            IO::MouseInputState &mouseState = ctx.input.getMouseState(inputControllerId);
            if (!mouseState.isLeftMouseButtonDown || mouseState.isMiddleMouseButtonDown
                || mouseState.isRightMouseButtonDown)
                continue;

            Physics::Ray ray = world.componentManagers.orbitCamera.getPickRay(orbitCameraId);

            glm::vec3 intersectionPoint;
            if (!world.componentManagers.terrainCollider.intersects(
                    terrainColliderInstanceId, ray, intersectionPoint))
                continue;

            newState.brushQuadX = (intersectionPoint.x / 127.5f) + 0.5f;
            newState.brushQuadY = (intersectionPoint.z / 127.5f) + 0.5f;
        }
    }

    void SceneWorld::render(EngineViewContext &vctx)
    {
        world.render(vctx);
    }
}}}}