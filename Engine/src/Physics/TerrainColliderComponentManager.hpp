#ifndef PHYSICS_TERRAINCOLLIDERCOMPONENTMANAGER_HPP
#define PHYSICS_TERRAINCOLLIDERCOMPONENTMANAGER_HPP

#include "../Common.hpp"

#include <vector>
#include "../Resources/TextureResource.hpp"

namespace Terrain { namespace Engine { namespace Physics {
    class EXPORT TerrainColliderComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            std::vector<int> entityId;
            std::vector<int> heightmapTextureResourceId;
            std::vector<int> rows;
            std::vector<int> columns;
            std::vector<float> patchSize;
            std::vector<int> firstHeightIndex;
            std::vector<float> patchHeights;
            std::vector<float> terrainHeight;

            ComponentData() : count(0)
            {
            }
        } data;

        float getTerrainPatchHeight(
            int x, int z, int &columns, int &rows, int &firstHeightIndex);

    public:
        TerrainColliderComponentManager();

        int create(int entityId,
            int heightmapTextureResourceId,
            int rows,
            int columns,
            float patchSize,
            float terrainHeight);

        void onTextureReloaded(Resources::TextureResourceData &resource);

        float getTerrainHeight(float worldX, float worldZ);
    };
}}}

#endif