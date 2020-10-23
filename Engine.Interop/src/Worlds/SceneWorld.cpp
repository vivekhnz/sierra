#include "SceneWorld.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    SceneWorld::SceneWorld(EngineContext &ctx) : ctx(ctx), world(ctx)
    {
    }

    void SceneWorld::initialize()
    {
        int terrainColumns = 256;
        int terrainRows = 256;
        float patchSize = 0.5f;
        float terrainHeight = 25.0f;

        const int RESOURCE_ID_TEXTURE_HEIGHTMAP = 0;
        const int RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED = 22;

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
        int terrainRendererInstanceId = world.componentManagers.terrainRenderer.create(
            entityId, RESOURCE_ID_TEXTURE_HEIGHTMAP, terrainRows, terrainColumns, patchSize,
            terrainHeight);
        world.componentManagers.terrainCollider.create(entityId, RESOURCE_ID_TEXTURE_HEIGHTMAP,
            terrainRows, terrainColumns, patchSize, terrainHeight);

        int &meshHandle =
            world.componentManagers.terrainRenderer.getMeshHandle(terrainRendererInstanceId);
        world.componentManagers.meshRenderer.create(entityId, meshHandle,
            RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED, materialUniformNames,
            materialUniformValues);
    }

    void SceneWorld::linkViewport(ViewportContext &vctx)
    {
        int cameraEntityId = ctx.entities.create();
        world.componentManagers.camera.create(cameraEntityId);

        int orbitCameraId = world.componentManagers.orbitCamera.create(cameraEntityId);
        world.componentManagers.orbitCamera.setPitch(orbitCameraId, glm::radians(15.0f));
        world.componentManagers.orbitCamera.setYaw(orbitCameraId, glm::radians(180.0f));
        world.componentManagers.orbitCamera.setDistance(orbitCameraId, 112.5f);
        world.componentManagers.orbitCamera.setInputControllerId(
            orbitCameraId, vctx.getInputControllerId());

        vctx.setCameraEntityId(cameraEntityId);
    }

    void SceneWorld::update(float deltaTime)
    {
        world.update(deltaTime);
    }

    void SceneWorld::render(EngineViewContext &vctx)
    {
        world.render(vctx);
    }
}}}}