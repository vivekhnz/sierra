#ifndef TERRAIN_PHYSICS_H
#define TERRAIN_PHYSICS_H

#include <glm/gtc/type_ptr.hpp>

#include "terrain_platform.h"

EXPORT bool physicsIsRayIntersectingTerrain(uint32 terrainColumns,
    uint32 terrainRows,
    float terrainPatchSize,
    float terrainHeight,
    float *patchHeights,
    glm::vec3 rayOrigin,
    glm::vec3 rayDirection,
    glm::vec3 &out_intersectionPoint);

#endif