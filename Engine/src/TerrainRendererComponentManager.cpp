#include "TerrainRendererComponentManager.hpp"

#include "TerrainResources.hpp"

namespace Terrain { namespace Engine {
    TerrainRendererComponentManager::TerrainRendererComponentManager(
        Graphics::Renderer &renderer, Graphics::MeshRendererComponentManager &meshRenderer) :
        renderer(renderer),
        meshRenderer(meshRenderer), calcTessLevelsShaderProgram(renderer), isInitialized(false)
    {
    }

    void TerrainRendererComponentManager::onShadersLoaded(
        const int count, Resources::ShaderResource *resources)
    {
        for (int i = 0; i < count; i++)
        {
            Resources::ShaderResource &resource = resources[i];
            if (resource.id != TerrainResources::RESOURCE_ID_SHADER_TERRAIN_COMPUTE_TESS_LEVEL)
                continue;

            std::vector<int> calcTessLevelShaderHandles;
            calcTessLevelShaderHandles.push_back(renderer.lookupShader(
                TerrainResources::RESOURCE_ID_SHADER_TERRAIN_COMPUTE_TESS_LEVEL));
            calcTessLevelsShaderProgram.link(calcTessLevelShaderHandles);
            calcTessLevelsShaderProgram.setFloat("targetTriangleSize", 0.015f);
            isInitialized = true;
            break;
        }
    }

    int TerrainRendererComponentManager::create(int entityId,
        int meshVertexBufferHandle,
        int rows,
        int columns,
        float patchSize,
        float terrainHeight)
    {
        data.entityId.push_back(entityId);
        data.meshVertexBufferHandle.push_back(meshVertexBufferHandle);
        data.rows.push_back(rows);
        data.columns.push_back(columns);
        data.patchSize.push_back(patchSize);
        data.terrainHeight.push_back(terrainHeight);
        data.isWireframeMode.push_back(false);

        // create buffer to store vertex edge data
        data.tessellationLevelBuffer.push_back(
            Graphics::Buffer(GL_SHADER_STORAGE_BUFFER, GL_STREAM_COPY));
        std::vector<glm::vec4> vertEdgeData(columns * rows * 10);
        data.tessellationLevelBuffer[data.count].fill(
            vertEdgeData.size() * sizeof(glm::vec4), vertEdgeData.data());

        return data.count++;
    }

    void TerrainRendererComponentManager::calculateTessellationLevels()
    {
        if (!isInitialized)
            return;

        for (int i = 0; i < data.count; i++)
        {
            int &rows = data.rows[i];
            int &columns = data.columns[i];
            int meshEdgeCount = (2 * (rows * columns)) - rows - columns;

            calcTessLevelsShaderProgram.setInt("horizontalEdgeCount", rows * (columns - 1));
            calcTessLevelsShaderProgram.setInt("columnCount", columns);

            glBindBufferBase(
                GL_SHADER_STORAGE_BUFFER, 0, data.tessellationLevelBuffer[i].getId());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1,
                renderer.getVertexBufferId(data.meshVertexBufferHandle[i]));
            glUseProgram(calcTessLevelsShaderProgram.getId());
            glDispatchCompute(meshEdgeCount, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }
    }

    void TerrainRendererComponentManager::updateMesh(
        int i, int heightmapWidth, int heightmapHeight, const void *heightmapData)
    {
        int &columns = data.columns[i];
        int &rows = data.rows[i];
        float &patchSize = data.patchSize[i];
        float &terrainHeight = data.terrainHeight[i];
        int &vertexBufferHandle = data.meshVertexBufferHandle[i];

        // update mesh vertices
        std::vector<float> vertices(columns * rows * 5);
        float offsetX = (columns - 1) * patchSize * -0.5f;
        float offsetY = (rows - 1) * patchSize * -0.5f;
        glm::vec2 uvSize = glm::vec2(1.0f / (columns - 1), 1.0f / (rows - 1));
        float xScalar = heightmapWidth / (float)columns;
        float yScalar = (heightmapWidth * heightmapHeight) / (float)rows;
        float heightScalar = terrainHeight / 65535.0f;
        const unsigned short *pixels = static_cast<const unsigned short *>(heightmapData);
        for (int y = 0; y < rows; y++)
        {
            int idxStart = y * columns;
            int rowStart = (int)(y * yScalar);
            float uvY = uvSize.y * y;

            for (int x = 0; x < columns; x++)
            {
                int idx = (idxStart + x) * 5;
                vertices[idx] = (x * patchSize) + offsetX;
                vertices[idx + 1] = pixels[rowStart + (int)(x * xScalar)] * heightScalar;
                vertices[idx + 2] = (y * patchSize) + offsetY;
                vertices[idx + 3] = uvSize.x * x;
                vertices[idx + 4] = uvY;
            }
        }
        renderer.updateVertexBuffer(
            vertexBufferHandle, vertices.size() * sizeof(float), vertices.data());

        // update heightmap size (used by adaptive tessellation)
        meshRenderer.setMaterialUniformVector2(meshRenderer.lookup(data.entityId[i]),
            "heightmapSize", glm::vec2(heightmapWidth, heightmapHeight));
    }

    void TerrainRendererComponentManager::toggleWireframeMode(
        int i, int terrainMaterialHandle, int wireframeMaterialHandle)
    {
        bool isWireframeMode = data.isWireframeMode[i];
        isWireframeMode = !isWireframeMode;
        data.isWireframeMode[i] = isWireframeMode;

        meshRenderer.setMaterialHandle(
            i, isWireframeMode ? wireframeMaterialHandle : terrainMaterialHandle);
    }

    TerrainRendererComponentManager::~TerrainRendererComponentManager()
    {
        data.count = 0;
    }
}}