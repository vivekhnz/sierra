#ifndef PHYSICS_RAY_HPP
#define PHYSICS_RAY_HPP

#include "../Common.hpp"

#include <glm/glm.hpp>

namespace Terrain { namespace Engine { namespace Physics {
    struct EXPORT Ray
    {
        glm::vec3 origin;
        glm::vec3 direction;
    };
}}}

#endif