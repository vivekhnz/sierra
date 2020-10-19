#include "Scene.hpp"

#include "TerrainResources.hpp"
#include "Graphics/Window.hpp"

namespace Terrain { namespace Engine {
    Scene::Scene(EngineContext &ctx, World &world) : ctx(ctx), world(world)
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

        // setup heightmap quad
        std::vector<float> quadVertices(20);

        quadVertices[0] = 0.0f;
        quadVertices[1] = 0.0f;
        quadVertices[2] = 0.0f;
        quadVertices[3] = 0.0f;
        quadVertices[4] = 0.0f;

        quadVertices[5] = 1.0f;
        quadVertices[6] = 0.0f;
        quadVertices[7] = 0.0f;
        quadVertices[8] = 1.0f;
        quadVertices[9] = 0.0f;

        quadVertices[10] = 1.0f;
        quadVertices[11] = 1.0f;
        quadVertices[12] = 0.0f;
        quadVertices[13] = 1.0f;
        quadVertices[14] = 1.0f;

        quadVertices[15] = 0.0f;
        quadVertices[16] = 1.0f;
        quadVertices[17] = 0.0f;
        quadVertices[18] = 0.0f;
        quadVertices[19] = 1.0f;

        std::vector<unsigned int> quadIndices(6);
        quadIndices[0] = 0;
        quadIndices[1] = 2;
        quadIndices[2] = 1;
        quadIndices[3] = 0;
        quadIndices[4] = 3;
        quadIndices[5] = 2;

        int quadMesh_meshHandle =
            ctx.assets.graphics.createMesh(GL_TRIANGLES, quadVertices, quadIndices);

        int quadMesh_entityId = ctx.entities.create();
        world.componentManagers.meshRenderer.create(quadMesh_entityId, quadMesh_meshHandle,
            TerrainResources::RESOURCE_ID_MATERIAL_QUAD, std::vector<std::string>(),
            std::vector<Graphics::UniformValue>());
    }

    void Scene::loadTerrainHeightmapFromFile(std::string path)
    {
        ctx.resources.reloadTexture(TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP, GL_R16,
            GL_RED, GL_UNSIGNED_SHORT, GL_MIRRORED_REPEAT, GL_LINEAR_MIPMAP_LINEAR, path,
            true);
    }

    void Scene::toggleTerrainWireframeMode()
    {
        world.componentManagers.terrainRenderer.toggleWireframeMode(
            terrain_terrainRendererInstanceId);
    }
}}