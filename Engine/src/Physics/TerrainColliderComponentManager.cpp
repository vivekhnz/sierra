#include "TerrainColliderComponentManager.hpp"

#include <algorithm>
#include <iterator>
#include <glm/glm.hpp>

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

    float barycentric(glm::vec3 a, glm::vec3 b, glm::vec3 c, float x, float y)
    {
        float det = (b.z - c.z) * (a.x - c.x) + (c.x - b.x) * (a.z - c.z);
        float l1 = ((b.z - c.z) * (x - c.x) + (c.x - b.x) * (y - c.z)) / det;
        float l2 = ((c.z - a.z) * (x - c.x) + (a.x - c.x) * (y - c.z)) / det;
        float l3 = 1.0f - l1 - l2;
        return (l1 * a.y) + (l2 * b.y) + (l3 * c.y);
    }

    float TerrainColliderComponentManager::getTerrainHeight(float worldX, float worldZ)
    {
        // we currently only support a single terrain collider
        if (data.count == 0)
            return 0.0f;

        int &columns = data.columns[0];
        int &rows = data.rows[0];
        float &patchSize = data.patchSize[0];
        int &firstHeightIndex = data.firstHeightIndex[0];

        float relativeX = worldX + (columns * patchSize * 0.5f);
        float relativeZ = worldZ + (rows * patchSize * 0.5f);
        float normalizedX = relativeX / patchSize;
        float normalizedZ = relativeZ / patchSize;
        int patchX = (int)floor(normalizedX);
        int patchZ = (int)floor(normalizedZ);
        float deltaX = normalizedX - patchX;
        float deltaZ = normalizedZ - patchZ;

        float topRight =
            getTerrainPatchHeight(patchX + 1, patchZ, columns, rows, firstHeightIndex);
        float bottomLeft =
            getTerrainPatchHeight(patchX, patchZ + 1, columns, rows, firstHeightIndex);
        if (deltaX <= 1.0f - deltaZ)
        {
            float topLeft =
                getTerrainPatchHeight(patchX, patchZ, columns, rows, firstHeightIndex);
            return barycentric(glm::vec3(0.0f, topLeft, 0.0f), glm::vec3(1.0f, topRight, 0.0f),
                glm::vec3(0.0f, bottomLeft, 1.0f), deltaX, deltaZ);
        }
        else
        {
            float bottomRight =
                getTerrainPatchHeight(patchX + 1, patchZ + 1, columns, rows, firstHeightIndex);
            return barycentric(glm::vec3(1.0f, topRight, 0.0f),
                glm::vec3(1.0f, bottomRight, 1.0f), glm::vec3(0.0f, bottomLeft, 1.0f), deltaX,
                deltaZ);
        }
    }

    float TerrainColliderComponentManager::getTerrainPatchHeight(
        int x, int z, int &columns, int &rows, int &firstHeightIndex)
    {
        int clampedX = std::clamp(x, 0, columns - 1);
        int clampedZ = std::clamp(z, 0, rows - 1);
        int i = (clampedZ * columns) + clampedX;
        return data.patchHeights[firstHeightIndex + i];
    }

    TerrainColliderComponentManager::~TerrainColliderComponentManager()
    {
        data.count = 0;
    }
}}}