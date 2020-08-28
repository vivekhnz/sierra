#ifndef GRAPHICS_MESHINSTANCE_HPP
#define GRAPHICS_MESHINSTANCE_HPP

#include "../Common.hpp"
#include "MeshData.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    struct EXPORT MeshInstance
    {
        int meshHandle;
        int shaderProgramId;
        int polygonMode;
    };
}}}

#endif