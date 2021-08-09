#ifndef ENGINE_HEIGHTFIELD_H
#define ENGINE_HEIGHTFIELD_H

#include <glm/gtc/type_ptr.hpp>

struct Heightfield
{
    uint32 vertsPerEdge;
    uint32 heightSamplesPerEdge;
    float maxHeight;
    float spaceBetweenVerts;
    float spaceBetweenHeightSamples;
    float *heights;
    glm::vec2 center;
};

#define HEIGHTFIELD_GET_HEIGHT(name) float name(Heightfield *heightfield, float worldX, float worldZ)
typedef HEIGHTFIELD_GET_HEIGHT(HeightfieldGetHeight);

#define HEIGHTFIELD_IS_RAY_INTERSECTING(name)                                                                     \
    bool name(Heightfield *heightfield, glm::vec3 rayOrigin, glm::vec3 rayDirection,                              \
        glm::vec3 *out_intersectionPoint, float *out_intersectionDistance)
typedef HEIGHTFIELD_IS_RAY_INTERSECTING(HeightfieldIsRayIntersecting);

#endif