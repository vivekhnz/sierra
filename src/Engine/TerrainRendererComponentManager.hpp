#ifndef TERRAINRENDERERCOMPONENTMANAGER_HPP
#define TERRAINRENDERERCOMPONENTMANAGER_HPP

#include "Common.hpp"

#include <vector>
#include "Graphics/MeshRendererComponentManager.hpp"
#include "Graphics/Renderer.hpp"
#include "Resources/TextureResource.hpp"

namespace Terrain { namespace Engine {
    class EXPORT TerrainRendererComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            std::vector<int> entityId;
            std::vector<int> heightmapTextureResourceId;
            std::vector<int> heightmapTextureHandle;
            std::vector<int> meshHandle;
            std::vector<int> meshVertexBufferHandle;
            std::vector<int> rows;
            std::vector<int> columns;
            std::vector<float> terrainHeight;
            std::vector<bool> isWireframeMode;
            std::vector<uint32> tessellationLevelBufferHandle;

            ComponentData() : count(0)
            {
            }
        } data;

        Graphics::Renderer &renderer;
        Graphics::MeshRendererComponentManager &meshRenderer;
        Graphics::GraphicsAssetManager &graphicsAssets;

    public:
        TerrainRendererComponentManager(Graphics::Renderer &renderer,
            Graphics::MeshRendererComponentManager &meshRenderer,
            Graphics::GraphicsAssetManager &graphicsAssets);

        int create(int entityId,
            int heightmapTextureResourceId,
            int heightmapTextureHandle,
            int rows,
            int columns,
            float patchSize,
            float terrainHeight);

        void onTextureReloaded(Resources::TextureResourceData &resource);

        void calculateTessellationLevels();
        void toggleWireframeMode(int i);
        int &getMeshHandle(int i)
        {
            return data.meshHandle[i];
        }
    };
}}

#endif