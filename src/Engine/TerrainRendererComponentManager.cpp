#include "TerrainRendererComponentManager.hpp"

#include "TerrainResources.hpp"
#include "terrain_renderer.h"
#include "terrain_assets.h"

namespace Terrain { namespace Engine {
    TerrainRendererComponentManager::TerrainRendererComponentManager(
        Graphics::Renderer &renderer,
        Graphics::MeshRendererComponentManager &meshRenderer,
        Graphics::GraphicsAssetManager &graphicsAssets) :
        renderer(renderer),
        meshRenderer(meshRenderer), graphicsAssets(graphicsAssets)
    {
    }

    int TerrainRendererComponentManager::create(int entityId,
        int heightmapTextureResourceId,
        int heightmapTextureHandle,
        int rows,
        int columns,
        float patchSize,
        float terrainHeight)
    {
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

        std::vector<Graphics::VertexAttribute> vertexAttributes(2);
        vertexAttributes[0] = Graphics::VertexAttribute::forFloat(3, false);
        vertexAttributes[1] = Graphics::VertexAttribute::forFloat(2, false);

        std::vector<Graphics::VertexBufferDescription> vertexBuffers(1);
        vertexBuffers[0] = {
            vertices.data(),                        // data
            (int)(vertices.size() * sizeof(float)), // size
            vertexAttributes.data(),                // attributes
            (int)vertexAttributes.size(),           // attributeCount
            false                                   // isPerInstance
        };

        int meshHandle = graphicsAssets.createMesh(GL_PATCHES, vertexBuffers, indices);
        int vertexBufferHandle = graphicsAssets.getMeshVertexBufferHandle(meshHandle, 0);

        data.entityId.push_back(entityId);
        data.heightmapTextureResourceId.push_back(heightmapTextureResourceId);
        data.heightmapTextureHandle.push_back(
            heightmapTextureHandle == -1 && heightmapTextureResourceId != -1
                ? renderer.lookupTexture(heightmapTextureResourceId)
                : heightmapTextureHandle);
        data.meshHandle.push_back(meshHandle);
        data.meshVertexBufferHandle.push_back(vertexBufferHandle);
        data.rows.push_back(rows);
        data.columns.push_back(columns);
        data.terrainHeight.push_back(terrainHeight);
        data.isWireframeMode.push_back(false);

        // create buffer to store vertex edge data
        uint32 tessLevelBufferHandle = rendererCreateBuffer(
            renderer.memory, RENDERER_SHADER_STORAGE_BUFFER, GL_STREAM_COPY);
        rendererUpdateBuffer(renderer.memory, tessLevelBufferHandle,
            columns * rows * 10 * sizeof(glm::vec4), 0);
        data.tessellationLevelBufferHandle.push_back(tessLevelBufferHandle);

        return data.count++;
    }

    void TerrainRendererComponentManager::onTextureReloaded(
        Resources::TextureResourceData &resource)
    {
        for (int i = 0; i < data.count; i++)
        {
            if (data.heightmapTextureResourceId[i] != resource.id)
                continue;

            data.heightmapTextureHandle[i] = renderer.lookupTexture(resource.id);
        }
    }

    void TerrainRendererComponentManager::calculateTessellationLevels()
    {
        ShaderProgramAsset *shaderProgramAsset = assetsGetShaderProgram(
            renderer.memory, ASSET_SHADER_PROGRAM_TERRAIN_CALC_TESS_LEVEL);
        if (!shaderProgramAsset)
            return;

        uint32 shaderProgramHandle = shaderProgramAsset->handle;
        rendererSetShaderProgramUniformFloat(
            renderer.memory, shaderProgramHandle, "targetTriangleSize", 0.015f);

        for (int i = 0; i < data.count; i++)
        {
            int &rows = data.rows[i];
            int &columns = data.columns[i];

            rendererSetShaderProgramUniformInteger(renderer.memory, shaderProgramHandle,
                "horizontalEdgeCount", rows * (columns - 1));
            rendererSetShaderProgramUniformInteger(
                renderer.memory, shaderProgramHandle, "columnCount", columns);
            rendererSetShaderProgramUniformFloat(
                renderer.memory, shaderProgramHandle, "terrainHeight", data.terrainHeight[i]);
            rendererBindTexture(renderer.memory, data.heightmapTextureHandle[i], 0);

            int meshEdgeCount = (2 * (rows * columns)) - rows - columns;

            rendererBindShaderStorageBuffer(
                renderer.memory, data.tessellationLevelBufferHandle[i], 0);
            rendererBindShaderStorageBuffer(
                renderer.memory, data.meshVertexBufferHandle[i], 1);
            rendererUseShaderProgram(renderer.memory, shaderProgramHandle);
            rendererDispatchCompute(meshEdgeCount, 1, 1);
            rendererShaderStorageMemoryBarrier();
        }
    }

    void TerrainRendererComponentManager::toggleWireframeMode(int i)
    {
        bool isWireframeMode = data.isWireframeMode[i];
        isWireframeMode = !isWireframeMode;
        data.isWireframeMode[i] = isWireframeMode;

        meshRenderer.setMaterial(i,
            isWireframeMode ? TerrainResources::Materials::TERRAIN_WIREFRAME
                            : TerrainResources::Materials::TERRAIN_TEXTURED);
    }
}}