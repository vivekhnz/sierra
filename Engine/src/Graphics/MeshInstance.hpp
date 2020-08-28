#ifndef GRAPHICS_MESHINSTANCE_HPP
#define GRAPHICS_MESHINSTANCE_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    struct EXPORT MeshInstance
    {
        int meshHandle;
        int materialHandle;
    };
}}}

#endif