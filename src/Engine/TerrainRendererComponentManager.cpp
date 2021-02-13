#include "TerrainRendererComponentManager.hpp"

#include "terrain_renderer.h"
#include "terrain_assets.h"

namespace Terrain { namespace Engine {
    TerrainRendererComponentManager::TerrainRendererComponentManager(
        Graphics::GraphicsAssetManager &graphicsAssets) :
        graphicsAssets(graphicsAssets)
    {
    }

    int TerrainRendererComponentManager::create(
        int entityId, int rows, int columns, float patchSize)
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

        data.entityId.push_back(entityId);
        data.meshHandle.push_back(meshHandle);

        return data.count++;
    }
}}