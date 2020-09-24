#ifndef GRAPHICS_MATERIAL_HPP
#define GRAPHICS_MATERIAL_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    struct EXPORT Material
    {
        int shaderProgramId;
        int polygonMode;

        int textureCount;
        int textureHandles[8];
    };
}}}

#endif