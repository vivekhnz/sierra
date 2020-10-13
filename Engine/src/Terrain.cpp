#include "Terrain.hpp"

#include <glm/glm.hpp>
#include "Graphics/Image.hpp"
#include "TerrainResources.hpp"

namespace Terrain { namespace Engine {
    Terrain::Terrain(EngineContext &ctx, World &world) :
        ctx(ctx), world(world), columns(256), rows(256), patchSize(0.5f), terrainHeight(25.0f)
    {
    }

    void Terrain::initialize()
    {
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
        terrainRendererInstanceId = world.componentManagers.terrainRenderer.create(
            entityId, rows, columns, patchSize, terrainHeight);
        colliderInstanceId = world.componentManagers.terrainCollider.create(
            entityId, columns, rows, patchSize, terrainHeight);

        int &meshHandle =
            world.componentManagers.terrainRenderer.getMeshHandle(terrainRendererInstanceId);
        world.componentManagers.meshRenderer.create(entityId, meshHandle,
            TerrainResources::RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED, materialUniformNames,
            materialUniformValues);
    }

    void Terrain::loadHeightmapFromFile(std::string path)
    {
        auto image = Graphics::Image(path, true);
        loadHeightmap(image.getWidth(), image.getHeight(), image.getData());
    }

    void Terrain::loadHeightmap(int textureWidth, int textureHeight, const void *data)
    {
        ctx.renderer.updateTexture(
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP),
            textureWidth, textureHeight, data);
        world.componentManagers.terrainCollider.updatePatchHeights(
            colliderInstanceId, textureWidth, textureHeight, data);
        world.componentManagers.terrainRenderer.updateMesh(
            terrainRendererInstanceId, textureWidth, textureHeight, data);
    }

    void Terrain::toggleWireframeMode()
    {
        world.componentManagers.terrainRenderer.toggleWireframeMode(terrainRendererInstanceId);
    }
}}