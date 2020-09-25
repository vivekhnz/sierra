#ifndef PHYSICS_TERRAINCOLLIDERCOMPONENTMANAGER_HPP
#define PHYSICS_TERRAINCOLLIDERCOMPONENTMANAGER_HPP

#include "../Common.hpp"
#include <vector>

namespace Terrain { namespace Engine { namespace Physics {
    class EXPORT TerrainColliderComponentManager
    {
    private:
        struct ComponentData
        {
            int count;
            std::vector<int> entityId;
            std::vector<int> columns;
            std::vector<int> rows;
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
        TerrainColliderComponentManager(const TerrainColliderComponentManager &that) = delete;
        TerrainColliderComponentManager &operator=(
            const TerrainColliderComponentManager &that) = delete;
        TerrainColliderComponentManager(TerrainColliderComponentManager &&) = delete;
        TerrainColliderComponentManager &operator=(
            TerrainColliderComponentManager &&) = delete;

        int create(int entityId, int columns, int rows, float patchSize, float terrainHeight);

        float getTerrainHeight(float worldX, float worldZ);
        void updatePatchHeights(
            int i, int heightmapWidth, int heightmapHeight, const void *heightmapData);

        ~TerrainColliderComponentManager();
    };
}}}

#endif