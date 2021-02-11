#include "terrain_physics.h"

bool isRayIntersectingBox(
    glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 boundsMin, glm::vec3 boundsMax)
{
    // based on:
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection

    glm::vec3 inverseRayDirection = 1.0f / rayDirection;

    bool xSign = inverseRayDirection.x >= 0;
    float txMin = ((xSign ? boundsMin.x : boundsMax.x) - rayOrigin.x) * inverseRayDirection.x;
    float txMax = ((xSign ? boundsMax.x : boundsMin.x) - rayOrigin.x) * inverseRayDirection.x;

    bool ySign = inverseRayDirection.y >= 0;
    float tyMin = ((ySign ? boundsMin.y : boundsMax.y) - rayOrigin.y) * inverseRayDirection.y;
    float tyMax = ((ySign ? boundsMax.y : boundsMin.y) - rayOrigin.y) * inverseRayDirection.y;

    if ((txMin > tyMax) || (tyMin > txMax))
        return false;

    if (tyMin > txMin)
        txMin = tyMin;

    if (tyMax < txMax)
        txMax = tyMax;

    bool zSign = inverseRayDirection.z >= 0;
    float tzMin = ((zSign ? boundsMin.z : boundsMax.z) - rayOrigin.z) * inverseRayDirection.z;
    float tzMax = ((zSign ? boundsMax.z : boundsMin.z) - rayOrigin.z) * inverseRayDirection.z;

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

bool isRayIntersectingTerrainSlice(glm::vec3 rayOrigin,
    glm::vec3 rayDirection,
    uint32 xStart,
    uint32 xEnd,
    uint32 yStart,
    uint32 yEnd,
    float offsetX,
    float offsetY,
    uint32 columns,
    float patchSize,
    float terrainHeight,
    float *patchHeights,
    float &inout_hitDistance)
{
    // raycast a bounding box first to avoid expensive triangle raycasting
    glm::vec3 boundsTopLeft =
        glm::vec3(offsetX + (xStart * patchSize), 0.0f, offsetY + (yStart * patchSize));
    glm::vec3 boundsBottomRight =
        glm::vec3(offsetX + (xEnd * patchSize), terrainHeight, offsetY + (yEnd * patchSize));
    if (!isRayIntersectingBox(rayOrigin, rayDirection, boundsTopLeft, boundsBottomRight))
    {
        return false;
    }

    bool hit = false;
    for (uint32 y = yStart; y < yEnd; y++)
    {
        for (uint32 x = xStart; x < xEnd; x++)
        {
            // calculate world-space corners of patch
            glm::vec3 topLeft = glm::vec3(offsetX + (x * patchSize),
                patchHeights[(y * columns) + x], offsetY + (y * patchSize));
            glm::vec3 topRight = glm::vec3(offsetX + ((x + 1) * patchSize),
                patchHeights[(y * columns) + x + 1], offsetY + (y * patchSize));
            glm::vec3 bottomRight = glm::vec3(offsetX + ((x + 1) * patchSize),
                patchHeights[((y + 1) * columns) + x + 1], offsetY + ((y + 1) * patchSize));
            glm::vec3 bottomLeft = glm::vec3(offsetX + (x * patchSize),
                patchHeights[((y + 1) * columns) + x], offsetY + ((y + 1) * patchSize));

            // raycast against 2 triangles of quad
            float intersectDist;
            if (isRayIntersectingTriangle(
                    rayOrigin, rayDirection, topLeft, topRight, bottomRight, intersectDist)
                && intersectDist < inout_hitDistance)
            {
                hit = true;
                inout_hitDistance = intersectDist;
            }
            if (isRayIntersectingTriangle(
                    rayOrigin, rayDirection, bottomRight, bottomLeft, topLeft, intersectDist)
                && intersectDist < inout_hitDistance)
            {
                hit = true;
                inout_hitDistance = intersectDist;
            }
        }
    }

    return hit;
}

bool physicsIsRayIntersectingTerrain(uint32 terrainColumns,
    uint32 terrainRows,
    float terrainPatchSize,
    float terrainHeight,
    float *patchHeights,
    glm::vec3 rayOrigin,
    glm::vec3 rayDirection,
    glm::vec3 &out_intersectionPoint)
{
    float offsetX = (terrainColumns - 1) * terrainPatchSize * -0.5f;
    float offsetY = (terrainRows - 1) * terrainPatchSize * -0.5f;

    float hitDistance = 999999.0f;
    bool hit = false;

    // divide terrain into 64 slices (8x8) and raycast against each slice's bounding box
    // this lets us skip expensive triangle raycasting if we don't hit the bounding box
    constexpr uint32 sliceCount = 8;
    uint32 colSlice = terrainColumns / sliceCount;
    uint32 rowSlice = terrainRows / sliceCount;

    for (uint32 y = 0; y < sliceCount; y++)
    {
        for (uint32 x = 0; x < sliceCount; x++)
        {
            uint32 xStart = x == 0 ? 0 : (colSlice * x) - 1;
            uint32 yStart = y == 0 ? 0 : (rowSlice * y) - 1;

            hit |= isRayIntersectingTerrainSlice(rayOrigin, rayDirection, xStart,
                (colSlice * (x + 1)) - 1, yStart, (rowSlice * (y + 1)) - 1, offsetX, offsetY,
                terrainColumns, terrainPatchSize, terrainHeight, patchHeights, hitDistance);
        }
    }

    if (hit)
    {
        out_intersectionPoint = rayOrigin + (rayDirection * hitDistance);
        return true;
    }
    return false;
}