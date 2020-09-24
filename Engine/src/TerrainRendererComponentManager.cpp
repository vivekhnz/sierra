#include "TerrainRendererComponentManager.hpp"

#include "IO/Path.hpp"

namespace Terrain { namespace Engine {
    TerrainRendererComponentManager::TerrainRendererComponentManager(
        Graphics::Renderer &renderer) :
        renderer(renderer)
    {
        std::vector<Graphics::Shader> calcTessLevelShaders;
        calcTessLevelShaders.push_back(renderer.shaders.loadComputeShaderFromFile(
            IO::Path::getAbsolutePath("data/terrain_calc_tess_levels_comp_shader.glsl")));
        calcTessLevelsShaderProgram.link(calcTessLevelShaders);
        calcTessLevelsShaderProgram.setFloat("targetTriangleSize", 0.015f);
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
    }

    TerrainRendererComponentManager::~TerrainRendererComponentManager()
    {
        data.count = 0;
    }
}}