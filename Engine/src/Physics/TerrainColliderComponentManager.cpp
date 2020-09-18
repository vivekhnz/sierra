#include "TerrainColliderComponentManager.hpp"

#include <iterator>

namespace Terrain { namespace Engine { namespace Physics {
    TerrainColliderComponentManager::TerrainColliderComponentManager()
    {
        data.count = 0;
    }

    int TerrainColliderComponentManager::create(
        int entityId, int columns, int rows, float patchSize)
    {
        data.entityId.push_back(entityId);
        data.columns.push_back(columns);
        data.rows.push_back(rows);
        data.patchSize.push_back(patchSize);
        data.firstHeightIndex.push_back(data.patchHeights.size());
        std::fill_n(std::back_inserter(data.patchHeights), columns * rows, 0.0f);
        return data.count++;
    }

    TerrainColliderComponentManager::~TerrainColliderComponentManager()
    {
        data.count = 0;
    }
}}}