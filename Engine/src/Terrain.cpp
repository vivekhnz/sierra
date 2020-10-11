#include "Terrain.hpp"

#include <glm/glm.hpp>
#include "Graphics/Image.hpp"
#include "TerrainResources.hpp"

namespace Terrain { namespace Engine {
    Terrain::Terrain(EngineContext &ctx, World &world) :
        ctx(ctx), world(world), columns(256), rows(256), patchSize(0.5f), terrainHeight(25.0f),
        mesh(ctx.renderer)
    {
    }

    void Terrain::initialize()
    {
        // build materials
        auto textureScale = glm::vec2(48.0f, 48.0f);

        std::vector<int> terrainMaterialTextureResourceIds(6);
        terrainMaterialTextureResourceIds[0] = TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP;
        terrainMaterialTextureResourceIds[1] = TerrainResources::RESOURCE_ID_TEXTURE_ALBEDO;
        terrainMaterialTextureResourceIds[2] = TerrainResources::RESOURCE_ID_TEXTURE_NORMAL;
        terrainMaterialTextureResourceIds[3] =
            TerrainResources::RESOURCE_ID_TEXTURE_DISPLACEMENT;
        terrainMaterialTextureResourceIds[4] = TerrainResources::RESOURCE_ID_TEXTURE_AO;
        terrainMaterialTextureResourceIds[5] = TerrainResources::RESOURCE_ID_TEXTURE_ROUGHNESS;

        std::vector<std::string> terrainMaterialUniformNames(7);
        terrainMaterialUniformNames[0] = "textureScale";
        terrainMaterialUniformNames[1] = "heightmapTexture";
        terrainMaterialUniformNames[2] = "albedoTexture";
        terrainMaterialUniformNames[3] = "normalTexture";
        terrainMaterialUniformNames[4] = "displacementTexture";
        terrainMaterialUniformNames[5] = "aoTexture";
        terrainMaterialUniformNames[6] = "roughnessTexture";

        std::vector<Graphics::UniformValue> terrainMaterialUniformValues(7);
        terrainMaterialUniformValues[0] = Graphics::UniformValue::forVector2(textureScale);
        terrainMaterialUniformValues[1] = Graphics::UniformValue::forInteger(0);
        terrainMaterialUniformValues[2] = Graphics::UniformValue::forInteger(1);
        terrainMaterialUniformValues[3] = Graphics::UniformValue::forInteger(2);
        terrainMaterialUniformValues[4] = Graphics::UniformValue::forInteger(3);
        terrainMaterialUniformValues[5] = Graphics::UniformValue::forInteger(4);
        terrainMaterialUniformValues[6] = Graphics::UniformValue::forInteger(5);

        terrainMaterialHandle = ctx.assets.graphics.createMaterial(
            TerrainResources::RESOURCE_ID_SHADER_PROGRAM_TERRAIN_TEXTURED, GL_FILL,
            terrainMaterialTextureResourceIds, terrainMaterialUniformNames,
            terrainMaterialUniformValues);

        std::vector<int> wireframeMaterialTextureResourceIds(6);
        wireframeMaterialTextureResourceIds[0] =
            TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP;
        wireframeMaterialTextureResourceIds[1] = TerrainResources::RESOURCE_ID_TEXTURE_ALBEDO;
        wireframeMaterialTextureResourceIds[2] = TerrainResources::RESOURCE_ID_TEXTURE_NORMAL;
        wireframeMaterialTextureResourceIds[3] =
            TerrainResources::RESOURCE_ID_TEXTURE_DISPLACEMENT;
        wireframeMaterialTextureResourceIds[4] = TerrainResources::RESOURCE_ID_TEXTURE_AO;
        wireframeMaterialTextureResourceIds[5] =
            TerrainResources::RESOURCE_ID_TEXTURE_ROUGHNESS;

        std::vector<std::string> wireframeMaterialUniformNames(4);
        wireframeMaterialUniformNames[0] = "color";
        wireframeMaterialUniformNames[1] = "heightmapTexture";
        wireframeMaterialUniformNames[2] = "displacementTexture";
        wireframeMaterialUniformNames[3] = "textureScale";

        std::vector<Graphics::UniformValue> wireframeMaterialUniformValues(4);
        wireframeMaterialUniformValues[0] =
            Graphics::UniformValue::forVector3(glm::vec3(0.0f, 1.0f, 0.0f));
        wireframeMaterialUniformValues[1] = Graphics::UniformValue::forInteger(0);
        wireframeMaterialUniformValues[2] = Graphics::UniformValue::forInteger(3);
        wireframeMaterialUniformValues[3] = Graphics::UniformValue::forVector2(textureScale);

        wireframeMaterialHandle = ctx.assets.graphics.createMaterial(
            TerrainResources::RESOURCE_ID_SHADER_PROGRAM_TERRAIN_WIREFRAME, GL_LINE,
            wireframeMaterialTextureResourceIds, wireframeMaterialUniformNames,
            wireframeMaterialUniformValues);

        // build mesh
        std::vector<float> vertices(columns * rows * 5);
        float offsetX = (columns - 1) * patchSize * -0.5f;
        float offsetY = (rows - 1) * patchSize * -0.5f;
        auto uvSize = glm::vec2(1.0f / (columns - 1), 1.0f / (rows - 1));
        for (int y = 0; y < rows; y++)
        {
            for (int x = 0; x < columns; x++)
            {
                int patchIndex = (y * columns) + x;
                int i = patchIndex * 5;
                vertices[i] = (x * patchSize) + offsetX;
                vertices[i + 1] = 0.0f;
                vertices[i + 2] = (y * patchSize) + offsetY;
                vertices[i + 3] = uvSize.x * x;
                vertices[i + 4] = uvSize.y * y;
            }
        }

        std::vector<unsigned int> indices((rows - 1) * (columns - 1) * 4);
        for (int y = 0; y < rows - 1; y++)
        {
            for (int x = 0; x < columns - 1; x++)
            {
                int vertIndex = (y * columns) + x;
                int elemIndex = ((y * (columns - 1)) + x) * 4;
                indices[elemIndex] = vertIndex;
                indices[elemIndex + 1] = vertIndex + columns;
                indices[elemIndex + 2] = vertIndex + columns + 1;
                indices[elemIndex + 3] = vertIndex + 1;
            }
        }
        mesh.initialize(vertices, indices);

        int meshHandle = ctx.resources.newMesh();
        Graphics::MeshData &meshData = ctx.resources.getMesh(meshHandle);
        meshData.vertexArrayId = mesh.getVertexArrayId();
        meshData.elementCount = indices.size();
        meshData.primitiveType = GL_PATCHES;

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
        colliderInstanceId = world.componentManagers.terrainCollider.create(
            entityId, columns, rows, patchSize, terrainHeight);
        meshRendererInstanceId = world.componentManagers.meshRenderer.create(entityId,
            meshHandle, terrainMaterialHandle, materialUniformNames, materialUniformValues);
        terrainRendererInstanceId = world.componentManagers.terrainRenderer.create(
            entityId, mesh.getVertexBufferHandle(), rows, columns, patchSize, terrainHeight);
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
        world.componentManagers.terrainRenderer.toggleWireframeMode(
            terrainRendererInstanceId, terrainMaterialHandle, wireframeMaterialHandle);
    }
}}