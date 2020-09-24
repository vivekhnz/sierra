#ifndef TERRAINRENDERERCOMPONENTMANAGER_HPP
#define TERRAINRENDERERCOMPONENTMANAGER_HPP

#include "Common.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/Renderer.hpp"
#include "Graphics/ShaderProgram.hpp"
#include <vector>

namespace Terrain { namespace Engine {
    class EXPORT TerrainRendererComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            std::vector<int> entityId;
            std::vector<int> meshVertexBufferHandle;
            std::vector<int> rows;
            std::vector<int> columns;
            std::vector<float> patchSize;
            std::vector<float> terrainHeight;
            std::vector<Graphics::Buffer> tessellationLevelBuffer;

            ComponentData() : count(0)
            {
            }
        } data;

        Graphics::Renderer &renderer;
        Graphics::ShaderProgram calcTessLevelsShaderProgram;

    public:
        TerrainRendererComponentManager(Graphics::Renderer &renderer);
        TerrainRendererComponentManager(const TerrainRendererComponentManager &that) = delete;
        TerrainRendererComponentManager &operator=(
            const TerrainRendererComponentManager &that) = delete;
        TerrainRendererComponentManager(TerrainRendererComponentManager &&) = delete;
        TerrainRendererComponentManager &operator=(
            TerrainRendererComponentManager &&) = delete;

        int create(int entityId,
            int meshVertexBufferHandle,
            int rows,
            int columns,
            float patchSize,
            float terrainHeight);

        void calculateTessellationLevels();
        void updateMesh(
            int i, int heightmapWidth, int heightmapHeight, const void *heightmapData);

        ~TerrainRendererComponentManager();
    };
}}

#endif