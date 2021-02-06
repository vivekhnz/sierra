#ifndef TERRAINRENDERERCOMPONENTMANAGER_HPP
#define TERRAINRENDERERCOMPONENTMANAGER_HPP

#include "Common.hpp"

#include <vector>
#include "Graphics/GraphicsAssetManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT TerrainRendererComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            std::vector<int> entityId;
            std::vector<int> meshHandle;

            ComponentData() : count(0)
            {
            }
        } data;

        Graphics::GraphicsAssetManager &graphicsAssets;

    public:
        TerrainRendererComponentManager(Graphics::GraphicsAssetManager &graphicsAssets);

        int create(int entityId, int rows, int columns, float patchSize);

        int &getMeshHandle(int i)
        {
            return data.meshHandle[i];
        }
    };
}}

#endif