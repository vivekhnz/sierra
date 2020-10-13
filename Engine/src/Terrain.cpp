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
        world.componentManagers.terrainCollider.create(entityId,
            TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP, columns, rows, patchSize,
            terrainHeight);

        int &meshHandle =
            world.componentManagers.terrainRenderer.getMeshHandle(terrainRendererInstanceId);
        world.componentManagers.meshRenderer.create(entityId, meshHandle,
            TerrainResources::RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED, materialUniformNames,
            materialUniformValues);
    }

    void Terrain::loadHeightmapFromFile(std::string path)
    {
        auto image = Graphics::Image(path, true);
        Resources::TextureResource resource = {
            TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP, // id
            GL_R16,                                          // internalFormat
            GL_RED,                                          // format
            GL_UNSIGNED_SHORT,                               // type
            GL_MIRRORED_REPEAT,                              // wrapMode
            GL_LINEAR_MIPMAP_LINEAR,                         // filterMode
            image.getWidth(),                                // width
            image.getHeight(),                               // height
            image.getData()                                  // data
        };

        loadHeightmap(resource);
    }

    void Terrain::loadHeightmap(Resources::TextureResource &resource)
    {
        ctx.renderer.onTextureReloaded(resource);
        world.componentManagers.terrainCollider.onTextureReloaded(resource);
        world.componentManagers.terrainRenderer.updateMesh(
            terrainRendererInstanceId, resource.width, resource.height, resource.data);
    }

    void Terrain::toggleWireframeMode()
    {
        world.componentManagers.terrainRenderer.toggleWireframeMode(terrainRendererInstanceId);
    }
}}