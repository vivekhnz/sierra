#ifndef TERRAIN_HEIGHTFIELD_H
#define TERRAIN_HEIGHTFIELD_H

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

EXPORT float heightfieldGetHeight(Heightfield *heightfield, float worldX, float worldZ);

EXPORT bool heightfieldIsRayIntersecting(Heightfield *heightfield,
    glm::vec3 rayOrigin,
    glm::vec3 rayDirection,
    glm::vec3 *out_intersectionPoint);

#endif