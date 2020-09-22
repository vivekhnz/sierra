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
            std::vector<unsigned int> meshVertexBufferId;
            std::vector<int> rows;
            std::vector<int> columns;
            std::vector<Graphics::Buffer> tessellationLevelBuffer;

            ComponentData() : count(0)
            {
            }
        } data;

        Graphics::ShaderProgram calcTessLevelsShaderProgram;

    public:
        TerrainRendererComponentManager(Graphics::Renderer &renderer);
        TerrainRendererComponentManager(const TerrainRendererComponentManager &that) = delete;
        TerrainRendererComponentManager &operator=(
            const TerrainRendererComponentManager &that) = delete;
        TerrainRendererComponentManager(TerrainRendererComponentManager &&) = delete;
        TerrainRendererComponentManager &operator=(
            TerrainRendererComponentManager &&) = delete;

        int create(int entityId, unsigned int meshVertexBufferId, int rows, int columns);

        void calculateTessellationLevels();

        ~TerrainRendererComponentManager();
    };
}}

#endif