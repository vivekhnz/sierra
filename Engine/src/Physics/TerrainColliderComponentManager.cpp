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

    void TerrainColliderComponentManager::updateHeights(
        int i, int width, int height, const unsigned short *pixels)
    {
        int &columns = data.columns[i];
        int &rows = data.rows[i];
        int &firstHeightIndex = data.firstHeightIndex[i];

        float xScalar = width / (float)columns;
        float yScalar = (height * width) / (float)rows;
        float heightScalar = data.terrainHeight[i] / 65535.0f;
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

    bool isRayIntersectingBox(
        glm::vec3 rayOrigin, glm::vec3 rayVector, glm::vec3 boundsMin, glm::vec3 boundsMax)
    {
        // based on:
        // https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection

        glm::vec3 invRayVector = 1.0f / rayVector;

        bool xSign = invRayVector.x >= 0;
        float txMin = ((xSign ? boundsMin.x : boundsMax.x) - rayOrigin.x) * invRayVector.x;
        float txMax = ((xSign ? boundsMax.x : boundsMin.x) - rayOrigin.x) * invRayVector.x;

        bool ySign = invRayVector.y >= 0;
        float tyMin = ((ySign ? boundsMin.y : boundsMax.y) - rayOrigin.y) * invRayVector.y;
        float tyMax = ((ySign ? boundsMax.y : boundsMin.y) - rayOrigin.y) * invRayVector.y;

        if ((txMin > tyMax) || (tyMin > txMax))
            return false;

        if (tyMin > txMin)
            txMin = tyMin;

        if (tyMax < txMax)
            txMax = tyMax;

        bool zSign = invRayVector.z >= 0;
        float tzMin = ((zSign ? boundsMin.z : boundsMax.z) - rayOrigin.z) * invRayVector.z;
        float tzMax = ((zSign ? boundsMax.z : boundsMin.z) - rayOrigin.z) * invRayVector.z;

        if ((txMin > tzMax) || (tzMin > txMax))
            return false;

        if (tzMin > txMin)
            txMin = tzMin;

        if (tzMax < txMax)
            txMax = tzMax;

        return true;
    }

    bool isRayIntersectingTriangle(glm::vec3 rayOrigin,
        glm::vec3 rayVector,
        glm::vec3 v1,
        glm::vec3 v2,
        glm::vec3 v3,
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

        out_intersectionDistance = t;
        return true;
    }

    bool TerrainColliderComponentManager::isRayIntersectingTerrainSlice(Ray &ray,
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
        float &inout_hitDistance)
    {
        // raycast a bounding box first to avoid expensive triangle raycasting
        glm::vec3 boundsTopLeft =
            glm::vec3(offsetX + (xStart * patchSize), 0.0f, offsetY + (yStart * patchSize));
        glm::vec3 boundsBottomRight = glm::vec3(
            offsetX + (xEnd * patchSize), terrainHeight, offsetY + (yEnd * patchSize));
        if (!isRayIntersectingBox(ray.origin, ray.direction, boundsTopLeft, boundsBottomRight))
        {
            return false;
        }

        bool hit = false;
        for (int y = yStart; y < yEnd; y++)
        {
            for (int x = xStart; x < xEnd; x++)
            {
                // calculate world-space corners of patch
                glm::vec3 topLeft = glm::vec3(offsetX + (x * patchSize),
                    data.patchHeights[firstHeightIndex + ((y * columns) + x)],
                    offsetY + (y * patchSize));
                glm::vec3 topRight = glm::vec3(offsetX + ((x + 1) * patchSize),
                    data.patchHeights[firstHeightIndex + ((y * columns) + x + 1)],
                    offsetY + (y * patchSize));
                glm::vec3 bottomRight = glm::vec3(offsetX + ((x + 1) * patchSize),
                    data.patchHeights[firstHeightIndex + (((y + 1) * columns) + x + 1)],
                    offsetY + ((y + 1) * patchSize));
                glm::vec3 bottomLeft = glm::vec3(offsetX + (x * patchSize),
                    data.patchHeights[firstHeightIndex + (((y + 1) * columns) + x)],
                    offsetY + ((y + 1) * patchSize));

                // raycast against 2 triangles of quad
                float intersectDist;
                if (isRayIntersectingTriangle(ray.origin, ray.direction, topLeft, topRight,
                        bottomRight, intersectDist)
                    && intersectDist < inout_hitDistance)
                {
                    hit = true;
                    inout_hitDistance = intersectDist;
                }
                if (isRayIntersectingTriangle(ray.origin, ray.direction, bottomRight,
                        bottomLeft, topLeft, intersectDist)
                    && intersectDist < inout_hitDistance)
                {
                    hit = true;
                    inout_hitDistance = intersectDist;
                }
            }
        }

        return hit;
    }

    bool TerrainColliderComponentManager::intersects(
        int i, Ray ray, glm::vec3 &out_intersectionPoint)
    {
        int &columns = data.columns[i];
        int &rows = data.rows[i];
        float &patchSize = data.patchSize[i];
        int &firstHeightIndex = data.firstHeightIndex[i];
        float &terrainHeight = data.terrainHeight[i];

        float offsetX = (columns - 1) * patchSize * -0.5f;
        float offsetY = (rows - 1) * patchSize * -0.5f;

        float hitDistance = 999999.0f;
        bool hit = false;

        // divide terrain into 64 slices (8x8) and raycast against each slice's bounding box
        // this lets us skip expensive triangle raycasting if we don't hit the bounding box
        int sliceCount = 8;
        int colSlice = columns / sliceCount;
        int rowSlice = rows / sliceCount;

        for (int y = 0; y < sliceCount; y++)
        {
            for (int x = 0; x < sliceCount; x++)
            {
                int xStart = x == 0 ? 0 : (colSlice * x) - 1;
                int yStart = y == 0 ? 0 : (rowSlice * y) - 1;

                hit |= isRayIntersectingTerrainSlice(ray, xStart, (colSlice * (x + 1)) - 1,
                    yStart, (rowSlice * (y + 1)) - 1, offsetX, offsetY, columns,
                    firstHeightIndex, patchSize, terrainHeight, hitDistance);
            }
        }

        if (hit)
        {
            out_intersectionPoint = ray.origin + (ray.direction * hitDistance);
            return true;
        }
        return false;
    }
}}}