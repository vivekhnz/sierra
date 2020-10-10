#ifndef GRAPHICS_MATERIAL_HPP
#define GRAPHICS_MATERIAL_HPP

#include "../Common.hpp"

#include <vector>
#include <string>
#include "UniformValue.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    struct EXPORT Material
    {
        int shaderProgramHandle;
        int polygonMode;

        int textureCount;
        int textureHandles[8];

        int uniformCount;
        std::vector<std::string> uniformNames;
        std::vector<UniformValue> uniformValues;
    };
}}}

#endif