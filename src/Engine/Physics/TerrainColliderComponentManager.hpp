#ifndef PHYSICS_TERRAINCOLLIDERCOMPONENTMANAGER_HPP
#define PHYSICS_TERRAINCOLLIDERCOMPONENTMANAGER_HPP

#include "../Common.hpp"

#include <vector>
#include "Ray.hpp"
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
        bool isRayIntersectingTerrainSlice(Ray &ray,
            int xStart,
            int xEnd,
            int yStart,
            int yEnd,
            float offsetX,
            float offsetY,
            int columns,
            int firstHeightIndex,
            float patchSize,
            float terrainHeight,
            float &inout_hitDistance);

    public:
        TerrainColliderComponentManager();

        int create(int entityId,
            int heightmapTextureResourceId,
            int rows,
            int columns,
            float patchSize,
            float terrainHeight);

        void onTextureReloaded(Resources::TextureResourceData &resource);
        void updateHeights(int i, int width, int height, const unsigned short *pixels);

        float getTerrainHeight(float worldX, float worldZ);
        bool intersects(int i, Ray ray, glm::vec3 &out_intersectionPoint);
    };
}}}

#endif