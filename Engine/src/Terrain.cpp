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
        // load shaders
        std::vector<int> terrainShaderResourceIds;
        terrainShaderResourceIds.push_back(
            TerrainResources::RESOURCE_ID_SHADER_TERRAIN_VERTEX);
        terrainShaderResourceIds.push_back(
            TerrainResources::RESOURCE_ID_SHADER_TERRAIN_TESS_CTRL);
        terrainShaderResourceIds.push_back(
            TerrainResources::RESOURCE_ID_SHADER_TERRAIN_TESS_EVAL);
        terrainShaderResourceIds.push_back(
            TerrainResources::RESOURCE_ID_SHADER_TERRAIN_FRAGMENT);

        std::vector<std::string> terrainShaderUniformNames;
        terrainShaderUniformNames.push_back("heightmapSize");
        terrainShaderUniformNames.push_back("heightmapTexture");
        terrainShaderUniformNames.push_back("albedoTexture");
        terrainShaderUniformNames.push_back("normalTexture");
        terrainShaderUniformNames.push_back("displacementTexture");
        terrainShaderUniformNames.push_back("aoTexture");
        terrainShaderUniformNames.push_back("roughnessTexture");
        terrainShaderUniformNames.push_back("terrainHeight");
        terrainShaderUniformNames.push_back("normalSampleOffset");
        terrainShaderUniformNames.push_back("textureScale");

        int terrainShaderProgramHandle = ctx.renderer.createShaderProgram(
            terrainShaderResourceIds, terrainShaderUniformNames);

        std::vector<int> wireframeShaderResourceIds;
        wireframeShaderResourceIds.push_back(
            TerrainResources::RESOURCE_ID_SHADER_WIREFRAME_VERTEX);
        wireframeShaderResourceIds.push_back(
            TerrainResources::RESOURCE_ID_SHADER_WIREFRAME_TESS_CTRL);
        wireframeShaderResourceIds.push_back(
            TerrainResources::RESOURCE_ID_SHADER_WIREFRAME_TESS_EVAL);
        wireframeShaderResourceIds.push_back(
            TerrainResources::RESOURCE_ID_SHADER_WIREFRAME_FRAGMENT);

        std::vector<std::string> wireframeShaderUniformNames;
        wireframeShaderUniformNames.push_back("heightmapSize");
        wireframeShaderUniformNames.push_back("heightmapTexture");
        wireframeShaderUniformNames.push_back("displacementTexture");
        wireframeShaderUniformNames.push_back("terrainHeight");
        wireframeShaderUniformNames.push_back("textureScale");
        wireframeShaderUniformNames.push_back("color");

        int wireframeShaderProgramHandle = ctx.renderer.createShaderProgram(
            wireframeShaderResourceIds, wireframeShaderUniformNames);

        // build materials
        auto textureScale = glm::vec2(48.0f, 48.0f);

        terrainMaterialHandle = ctx.resources.newMaterial();
        Graphics::Material &terrainMaterial = ctx.resources.getMaterial(terrainMaterialHandle);
        terrainMaterial.shaderProgramHandle = terrainShaderProgramHandle;
        terrainMaterial.polygonMode = GL_FILL;
        terrainMaterial.textureCount = 6;
        terrainMaterial.textureHandles[0] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP);
        terrainMaterial.textureHandles[1] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_ALBEDO);
        terrainMaterial.textureHandles[2] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_NORMAL);
        terrainMaterial.textureHandles[3] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_DISPLACEMENT);
        terrainMaterial.textureHandles[4] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_AO);
        terrainMaterial.textureHandles[5] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_ROUGHNESS);
        terrainMaterial.uniformCount = 7;
        terrainMaterial.uniformNames.push_back("textureScale");
        terrainMaterial.uniformValues.push_back(
            Graphics::UniformValue::forVector2(textureScale));
        terrainMaterial.uniformNames.push_back("heightmapTexture");
        terrainMaterial.uniformValues.push_back(Graphics::UniformValue::forInteger(0));
        terrainMaterial.uniformNames.push_back("albedoTexture");
        terrainMaterial.uniformValues.push_back(Graphics::UniformValue::forInteger(1));
        terrainMaterial.uniformNames.push_back("normalTexture");
        terrainMaterial.uniformValues.push_back(Graphics::UniformValue::forInteger(2));
        terrainMaterial.uniformNames.push_back("displacementTexture");
        terrainMaterial.uniformValues.push_back(Graphics::UniformValue::forInteger(3));
        terrainMaterial.uniformNames.push_back("aoTexture");
        terrainMaterial.uniformValues.push_back(Graphics::UniformValue::forInteger(4));
        terrainMaterial.uniformNames.push_back("roughnessTexture");
        terrainMaterial.uniformValues.push_back(Graphics::UniformValue::forInteger(5));

        wireframeMaterialHandle = ctx.resources.newMaterial();
        Graphics::Material &wireframeMaterial =
            ctx.resources.getMaterial(wireframeMaterialHandle);
        wireframeMaterial.shaderProgramHandle = wireframeShaderProgramHandle;
        wireframeMaterial.polygonMode = GL_LINE;
        wireframeMaterial.textureCount = 6;
        wireframeMaterial.textureHandles[0] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_HEIGHTMAP);
        wireframeMaterial.textureHandles[1] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_ALBEDO);
        wireframeMaterial.textureHandles[2] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_NORMAL);
        wireframeMaterial.textureHandles[3] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_DISPLACEMENT);
        wireframeMaterial.textureHandles[4] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_AO);
        wireframeMaterial.textureHandles[5] =
            ctx.renderer.lookupTexture(TerrainResources::RESOURCE_ID_TEXTURE_ROUGHNESS);
        wireframeMaterial.uniformCount = 4;
        wireframeMaterial.uniformNames.push_back("color");
        wireframeMaterial.uniformValues.push_back(
            Graphics::UniformValue::forVector3(glm::vec3(0.0f, 1.0f, 0.0f)));
        wireframeMaterial.uniformNames.push_back("heightmapTexture");
        wireframeMaterial.uniformValues.push_back(Graphics::UniformValue::forInteger(0));
        wireframeMaterial.uniformNames.push_back("displacementTexture");
        wireframeMaterial.uniformValues.push_back(Graphics::UniformValue::forInteger(3));
        wireframeMaterial.uniformNames.push_back("textureScale");
        wireframeMaterial.uniformValues.push_back(
            Graphics::UniformValue::forVector2(textureScale));

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