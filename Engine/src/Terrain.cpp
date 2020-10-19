#include "Terrain.hpp"

#include <glm/glm.hpp>
#include "TerrainResources.hpp"

namespace Terrain { namespace Engine {
    Terrain::Terrain(EngineContext &ctx, World &world) : ctx(ctx), world(world)
    {
        int columns = 256;
        int rows = 256;
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
            glm::vec2(1.0f / (patchSize * columns), 1.0f / (patchSize * rows)));

        // create entity and components
        int entityId = ctx.entities.create();
        terrainRendererInstanceId = world.componentManagers.terrainRenderer.create(entityId,
            TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP, rows, columns, patchSize,
            terrainHeight);
        world.componentManagers.terrainCollider.create(entityId,
            TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP, rows, columns, patchSize,
            terrainHeight);

        int &meshHandle =
            world.componentManagers.terrainRenderer.getMeshHandle(terrainRendererInstanceId);
        world.componentManagers.meshRenderer.create(entityId, meshHandle,
            TerrainResources::RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED, materialUniformNames,
            materialUniformValues);
    }

    void Terrain::loadHeightmapFromFile(std::string path)
    {
        ctx.resources.reloadTexture(TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP, GL_R16,
            GL_RED, GL_UNSIGNED_SHORT, GL_MIRRORED_REPEAT, GL_LINEAR_MIPMAP_LINEAR, path,
            true);
    }

    void Terrain::toggleWireframeMode()
    {
        world.componentManagers.terrainRenderer.toggleWireframeMode(terrainRendererInstanceId);
    }
}}