#ifndef GRAPHICS_SHADERPROGRAM_HPP
#define GRAPHICS_SHADERPROGRAM_HPP

#include "../Common.hpp"
#include <glad/glad.h>
#include <vector>
#include "Renderer.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT ShaderProgram
    {
        int handle;
        Renderer &renderer;

    public:
        ShaderProgram(Renderer &renderer);

        int getHandle() const;
        void link(const std::vector<int> &shaderHandles);
    };
}}}

#endif