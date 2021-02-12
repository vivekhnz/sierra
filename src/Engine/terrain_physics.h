#ifndef TERRAIN_PHYSICS_H
#define TERRAIN_PHYSICS_H

#include <glm/gtc/type_ptr.hpp>

#include "terrain_platform.h"

struct Heightfield
{
    uint32 columns;
    uint32 rows;
    float maxHeight;
    float spacing;
    float *heights;
    glm::vec2 position;
};

EXPORT bool physicsIsRayIntersectingHeightfield(Heightfield *heightfield,
    glm::vec3 rayOrigin,
    glm::vec3 rayDirection,
    glm::vec3 &out_intersectionPoint);

#endif