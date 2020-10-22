#include "Scene.hpp"

#include "TerrainResources.hpp"
#include "Graphics/Window.hpp"

namespace Terrain { namespace Engine {
    Scene::Scene(EngineContext &ctx, World &world) : world(world)
    {
        int terrainColumns = 256;
        int terrainRows = 256;
        float patchSize = 0.5f;
        float terrainHeight = 25.0f;

        // build material uniforms
        std::vector<std::string> materialUniformNames(3);
        materialUniformNames[0] = "terrainHeight";
        materialUniformNames[1] = "heightmapSize";
        materialUniformNames[2] = "normalSampleOffset";

        std::vector<Graphics::UniformValue> materialUniformValues(3);
        materialUniformValues[0] = Graphics::UniformValue::forFloat(terrainHeight);
        materialUniformValues[1] = Graphics::UniformValue::forVector2(glm::vec2(1.0f, 1.0f));
        materialUniformValues[2] = Graphics::UniformValue::forVector2(
            glm::vec2(1.0f / (patchSize * terrainColumns), 1.0f / (patchSize * terrainRows)));

        // create entity and components
        int entityId = ctx.entities.create();
        terrain_terrainRendererInstanceId = world.componentManagers.terrainRenderer.create(
            entityId, TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP, terrainRows,
            terrainColumns, patchSize, terrainHeight);
        world.componentManagers.terrainCollider.create(entityId,
            TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP, terrainRows, terrainColumns,
            patchSize, terrainHeight);

        int &meshHandle = world.componentManagers.terrainRenderer.getMeshHandle(
            terrain_terrainRendererInstanceId);
        world.componentManagers.meshRenderer.create(entityId, meshHandle,
            TerrainResources::RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED, materialUniformNames,
            materialUniformValues);

        // configure input
        ctx.input.mapCommand(GLFW_KEY_Z, std::bind(&Scene::toggleTerrainWireframeMode, this));
    }

    void Scene::toggleTerrainWireframeMode()
    {
        world.componentManagers.terrainRenderer.toggleWireframeMode(
            terrain_terrainRendererInstanceId);
    }
}}