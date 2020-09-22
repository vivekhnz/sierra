#include "TerrainRendererComponentManager.hpp"

#include "IO/Path.hpp"

namespace Terrain { namespace Engine {
    TerrainRendererComponentManager::TerrainRendererComponentManager(
        Graphics::Renderer &renderer)
    {
        std::vector<Graphics::Shader> calcTessLevelShaders;
        calcTessLevelShaders.push_back(renderer.shaders.loadComputeShaderFromFile(
            IO::Path::getAbsolutePath("data/terrain_calc_tess_levels_comp_shader.glsl")));
        calcTessLevelsShaderProgram.link(calcTessLevelShaders);
        calcTessLevelsShaderProgram.setFloat("targetTriangleSize", 0.015f);
    }

    int TerrainRendererComponentManager::create(
        int entityId, unsigned int meshVertexBufferId, int rows, int columns)
    {
        data.entityId.push_back(entityId);
        data.meshVertexBufferId.push_back(meshVertexBufferId);
        data.rows.push_back(rows);
        data.columns.push_back(columns);

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
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, data.meshVertexBufferId[i]);
            glUseProgram(calcTessLevelsShaderProgram.getId());
            glDispatchCompute(meshEdgeCount, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }
    }

    TerrainRendererComponentManager::~TerrainRendererComponentManager()
    {
        data.count = 0;
    }
}}