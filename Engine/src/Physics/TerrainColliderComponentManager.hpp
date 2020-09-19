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

        int create(int entityId, int columns, int rows, float patchSize);

        int getFirstHeightIndex(int i)
        {
            return data.firstHeightIndex[i];
        }
        void setPatchHeight(int index, float value)
        {
            data.patchHeights[index] = value;
        }

        float getTerrainHeight(float worldX, float worldZ);

        ~TerrainColliderComponentManager();
    };
}}}

#endif