#include "TerrainColliderComponentManager.hpp"

#include <algorithm>
#include <iterator>

namespace Terrain { namespace Engine { namespace Physics {
    TerrainColliderComponentManager::TerrainColliderComponentManager()
    {
        data.count = 0;
    }

    int TerrainColliderComponentManager::create(int entityId,
        int heightmapTextureResourceId,
        int rows,
        int columns,
        float patchSize,
        float terrainHeight)
    {
        data.entityId.push_back(entityId);
        data.heightmapTextureResourceId.push_back(heightmapTextureResourceId);
        data.rows.push_back(rows);
        data.columns.push_back(columns);
        data.patchSize.push_back(patchSize);
        data.firstHeightIndex.push_back(data.patchHeights.size());
        std::fill_n(std::back_inserter(data.patchHeights), columns * rows, 0.0f);
        data.terrainHeight.push_back(terrainHeight);
        return data.count++;
    }

    void TerrainColliderComponentManager::onTextureReloaded(
        Resources::TextureResourceData &resource)
    {
        for (int i = 0; i < data.count; i++)
        {
            if (data.heightmapTextureResourceId[i] != resource.id)
                continue;

            int &columns = data.columns[i];
            int &rows = data.rows[i];
            int &firstHeightIndex = data.firstHeightIndex[i];

            float xScalar = resource.width / (float)columns;
            float yScalar = (resource.height * resource.width) / (float)rows;
            float heightScalar = data.terrainHeight[i] / 65535.0f;
            const unsigned short *pixels = static_cast<const unsigned short *>(resource.data);
            for (int y = 0; y < rows; y++)
            {
                int idxStart = firstHeightIndex + (y * columns);
                int rowStart = (int)(y * yScalar);

                for (int x = 0; x < columns; x++)
                {
                    data.patchHeights[idxStart + x] =
                        pixels[rowStart + (int)(x * xScalar)] * heightScalar;
                }
            }
        }
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

    bool isRayIntersectingTriangle(glm::vec3 rayOrigin,
        glm::vec3 rayVector,
        glm::vec3 v1,
        glm::vec3 v2,
        glm::vec3 v3,
        glm::vec3 &out_intersectionPoint,
        float &out_intersectionDistance)
    {
        // Moller-Trumbore intersection algorithm
        // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

        const float EPSILON = 0.0000001f;
        glm::vec3 h, s, q;
        float a, f, u, v;
        glm::vec3 edge1 = v2 - v1;
        glm::vec3 edge2 = v3 - v1;
        h = glm::cross(rayVector, edge2);
        a = glm::dot(edge1, h);
        if (a > -EPSILON && a < EPSILON)
            return false; // ray is parallel to this triangle

        f = 1.0 / a;
        s = rayOrigin - v1;
        u = f * glm::dot(s, h);
        if (u < 0.0 || u > 1.0)
            return false;
        q = glm::cross(s, edge1);
        v = f * glm::dot(rayVector, q);
        if (v < 0.0 || u + v > 1.0)
            return false;

        // compute t to find out where the intersection point is on the line
        float t = f * glm::dot(edge2, q);
        if (t <= EPSILON)
            return false; // there is a line intersection but not a ray intersection

        out_intersectionPoint = rayOrigin + (rayVector * t);
        out_intersectionDistance = t;
        return true;
    }

    bool TerrainColliderComponentManager::intersects(
        int i, Ray ray, glm::vec3 &out_intersectionPoint)
    {
        int &columns = data.columns[i];
        int &rows = data.rows[i];
        float &patchSize = data.patchSize[i];

        // calculate world-space corners of terrain quad
        float offsetX = (columns - 1) * patchSize * -0.5f;
        float offsetY = (rows - 1) * patchSize * -0.5f;

        glm::vec3 topLeft = glm::vec3(offsetX, 0.0f, offsetY);
        glm::vec3 topRight = glm::vec3(((columns - 1) * patchSize) + offsetX, 0.0f, offsetY);
        glm::vec3 bottomRight = glm::vec3(
            ((columns - 1) * patchSize) + offsetX, 0.0f, ((rows - 1) * patchSize) + offsetY);
        glm::vec3 bottomLeft =
            glm::vec3(0 + offsetX, 0.0f, ((rows - 1) * patchSize) + offsetY);

        // raycast against 2 triangles of terrain quad
        glm::vec3 intersectPointA, intersectPointB;
        float intersectDistA, intersectDistB;
        bool hitA = isRayIntersectingTriangle(ray.origin, ray.direction, topLeft, topRight,
            bottomRight, intersectPointA, intersectDistA);
        bool hitB = isRayIntersectingTriangle(ray.origin, ray.direction, bottomRight,
            bottomLeft, topLeft, intersectPointB, intersectDistB);
        if (hitA && hitB)
        {
            // use the result from the raycast hit closest to the ray's origin
            out_intersectionPoint =
                intersectDistA < intersectDistB ? intersectPointA : intersectPointB;
            return true;
        }
        else if (hitA)
        {
            out_intersectionPoint = intersectPointA;
            return true;
        }
        else if (hitB)
        {
            out_intersectionPoint = intersectPointB;
            return true;
        }
        return false;
    }
}}}