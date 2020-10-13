#ifndef TERRAINRENDERERCOMPONENTMANAGER_HPP
#define TERRAINRENDERERCOMPONENTMANAGER_HPP

#include "Common.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/MeshRendererComponentManager.hpp"
#include "Graphics/Renderer.hpp"
#include <vector>

namespace Terrain { namespace Engine {
    class EXPORT TerrainRendererComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            std::vector<int> entityId;
            std::vector<int> meshHandle;
            std::vector<int> meshVertexBufferHandle;
            std::vector<int> rows;
            std::vector<int> columns;
            std::vector<float> patchSize;
            std::vector<float> terrainHeight;
            std::vector<bool> isWireframeMode;
            std::vector<Graphics::Buffer> tessellationLevelBuffer;

            ComponentData() : count(0)
            {
            }
        } data;

        Graphics::Renderer &renderer;
        Graphics::MeshRendererComponentManager &meshRenderer;
        Graphics::GraphicsAssetManager &graphicsAssets;

        int calcTessLevelsShaderProgramHandle;

    public:
        TerrainRendererComponentManager(Graphics::Renderer &renderer,
            Graphics::MeshRendererComponentManager &meshRenderer,
            Graphics::GraphicsAssetManager &graphicsAssets);
        TerrainRendererComponentManager(const TerrainRendererComponentManager &that) = delete;
        TerrainRendererComponentManager &operator=(
            const TerrainRendererComponentManager &that) = delete;
        TerrainRendererComponentManager(TerrainRendererComponentManager &&) = delete;
        TerrainRendererComponentManager &operator=(
            TerrainRendererComponentManager &&) = delete;

        void onShaderProgramsLoaded(
            const int count, Resources::ShaderProgramResource *resources);

        int create(int entityId, int rows, int columns, float patchSize, float terrainHeight);

        void calculateTessellationLevels();
        void updateMesh(
            int i, int heightmapWidth, int heightmapHeight, const void *heightmapData);
        void toggleWireframeMode(int i);
        int &getMeshHandle(int i)
        {
            return data.meshHandle[i];
        }

        ~TerrainRendererComponentManager();
    };
}}

#endif