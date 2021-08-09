#include "engine_heightfield.h"

float barycentric(glm::vec3 a, glm::vec3 b, glm::vec3 c, float x, float y)
{
    float det = (b.z - c.z) * (a.x - c.x) + (c.x - b.x) * (a.z - c.z);
    float l1 = ((b.z - c.z) * (x - c.x) + (c.x - b.x) * (y - c.z)) / det;
    float l2 = ((c.z - a.z) * (x - c.x) + (a.x - c.x) * (y - c.z)) / det;
    float l3 = 1.0f - l1 - l2;
    return (l1 * a.y) + (l2 * b.y) + (l3 * c.y);
}

glm::vec2 getOrigin(Heightfield *heightfield)
{
    float tileLengthInWorldUnits =
        (heightfield->heightSamplesPerEdge - 1) * heightfield->spaceBetweenHeightSamples;
    glm::vec2 result = heightfield->center - (tileLengthInWorldUnits * 0.5f);
    return result;
}

HEIGHTFIELD_GET_HEIGHT(heightfieldGetHeight)
{
    glm::vec2 posWorldSpace = glm::vec2(worldX, worldZ);
    glm::vec2 origin = getOrigin(heightfield);
    glm::vec2 posObjectSpace = posWorldSpace - origin;

    glm::vec2 posSampleSpace = posObjectSpace / heightfield->spaceBetweenHeightSamples;
    glm::vec2 posSampleSpaceFloor = glm::floor(posSampleSpace);
    glm::vec2 delta = posSampleSpace - posSampleSpaceFloor;

    uint32 heightSamplesPerEdge = heightfield->heightSamplesPerEdge;
    int32 x = (int32)posSampleSpaceFloor.x;
    int32 y = (int32)posSampleSpaceFloor.y;

    if (x < -1 || y < -1 || x >= heightSamplesPerEdge || y >= heightSamplesPerEdge)
    {
        return 0.0f;
    }
    else
    {
        float *topLeftSample = &heightfield->heights[(y * heightSamplesPerEdge) + x];
        glm::vec3 topLeft = glm::vec3(0, x < 0 ? 0 : *topLeftSample, 0);
        glm::vec3 topRight = glm::vec3(1, x + 1 >= heightSamplesPerEdge ? 0 : *(topLeftSample + 1), 0);
        glm::vec3 bottomLeft = glm::vec3(0, y < 0 ? 0 : *(topLeftSample + heightSamplesPerEdge), 1);
        glm::vec3 bottomRight = glm::vec3(1,
            x + 1 >= heightSamplesPerEdge || y + 1 >= heightSamplesPerEdge
                ? 0
                : *(topLeftSample + heightSamplesPerEdge + 1),
            1);

        return delta.x <= 1.0f - delta.y ? barycentric(topLeft, topRight, bottomLeft, delta.x, delta.y)
                                         : barycentric(topRight, bottomRight, bottomLeft, delta.x, delta.y);
    }
}

bool isRayIntersectingBox(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 boundsMin, glm::vec3 boundsMax)
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

bool isRayIntersectingHeightfieldSlice(Heightfield *heightfield,
    glm::vec3 rayOrigin,
    glm::vec3 rayDirection,
    uint32 xStart,
    uint32 xEnd,
    uint32 yStart,
    uint32 yEnd,
    float *inout_hitDistance)
{
    glm::vec2 origin = getOrigin(heightfield);
    glm::vec3 origin3 = glm::vec3(origin.x, 0, origin.y);
    uint32 heightSamplesPerEdge = heightfield->heightSamplesPerEdge;
    float distBetweenHeightSamples = heightfield->spaceBetweenHeightSamples;

    // raycast a bounding box first to avoid expensive triangle raycasting
    glm::vec3 boundsTopLeft =
        origin3 + glm::vec3(xStart * distBetweenHeightSamples, 0, yStart * distBetweenHeightSamples);
    glm::vec3 boundsBottomRight = origin3
        + glm::vec3(xEnd * distBetweenHeightSamples, heightfield->maxHeight, yEnd * distBetweenHeightSamples);
    if (!isRayIntersectingBox(rayOrigin, rayDirection, boundsTopLeft, boundsBottomRight))
    {
        return false;
    }

    bool hit = false;
    for (uint32 y = yStart; y < yEnd; y++)
    {
        float top = y * distBetweenHeightSamples;
        float bottom = top + distBetweenHeightSamples;

        for (uint32 x = xStart; x < xEnd; x++)
        {
            float left = x * distBetweenHeightSamples;
            float right = left + distBetweenHeightSamples;

            // calculate world-space corners of patch
            float *topLeftSample = &heightfield->heights[(y * heightSamplesPerEdge) + x];
            glm::vec3 topLeft = origin3 + glm::vec3(left, *topLeftSample, top);
            glm::vec3 topRight = origin3 + glm::vec3(right, *(topLeftSample + 1), top);
            glm::vec3 bottomLeft = origin3 + glm::vec3(left, *(topLeftSample + heightSamplesPerEdge), bottom);
            glm::vec3 bottomRight =
                origin3 + glm::vec3(right, *(topLeftSample + heightSamplesPerEdge + 1), bottom);

            // raycast against 2 triangles of quad
            float intersectDist;
            if (isRayIntersectingTriangle(rayOrigin, rayDirection, topLeft, topRight, bottomRight, intersectDist)
                && intersectDist < *inout_hitDistance)
            {
                hit = true;
                *inout_hitDistance = intersectDist;
            }
            if (isRayIntersectingTriangle(rayOrigin, rayDirection, bottomRight, bottomLeft, topLeft, intersectDist)
                && intersectDist < *inout_hitDistance)
            {
                hit = true;
                *inout_hitDistance = intersectDist;
            }
        }
    }

    return hit;
}

HEIGHTFIELD_IS_RAY_INTERSECTING(heightfieldIsRayIntersecting)
{
    *out_intersectionDistance = 999999.0f;
    bool hit = false;

#if 0
    uint32 heightSamplesPerEdge = heightfield->columns;
    uint32 maxSample = heightSamplesPerEdge - 2;

    // divide heightfield into 64 slices (8x8) and raycast against each slice's bounding box
    // this lets us skip expensive triangle raycasting if we don't hit the bounding box
    constexpr uint32 sliceCount = 8;
    assert(maxSample < sliceCount || maxSample % sliceCount == 0);
    assert(heightfield->rows < sliceCount || heightfield->rows % sliceCount == 0);

    uint32 colSlice = maxSample < sliceCount ? maxSample : maxSample / sliceCount;
    uint32 rowSlice = heightfield->rows < sliceCount ? heightfield->rows : heightfield->rows / sliceCount;

    uint32 yStart = 0;
    uint32 yEnd = rowSlice - 1;
    while (yEnd < maxSample)
    {
        uint32 xStart = 0;
        uint32 xEnd = colSlice - 1;
        while (xEnd < maxSample)
        {
            hit |= isRayIntersectingHeightfieldSlice(
                heightfield, rayOrigin, rayDirection, xStart, xEnd, yStart, yEnd, out_intersectionDistance);

            xStart = xEnd;
            xEnd += colSlice;
        }

        yStart = yEnd;
        yEnd += rowSlice;
    }
#else
    uint32 heightSamplesPerEdge = heightfield->heightSamplesPerEdge;
    hit = isRayIntersectingHeightfieldSlice(heightfield, rayOrigin, rayDirection, 0, heightSamplesPerEdge - 2, 0,
        heightSamplesPerEdge - 2, out_intersectionDistance);
#endif

    if (hit)
    {
        *out_intersectionPoint = rayOrigin + (rayDirection * *out_intersectionDistance);
        return true;
    }
    return false;
}