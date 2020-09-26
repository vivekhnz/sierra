#ifndef GRAPHICS_SHADER_HPP
#define GRAPHICS_SHADER_HPP

#include "../Common.hpp"
#include <glad/glad.h>
#include <string>

namespace Terrain { namespace Engine { namespace Graphics {
    class Renderer;

    class EXPORT Shader
    {
        unsigned int id;

        Renderer &renderer;

    public:
        Shader(Renderer &renderer, GLenum shaderType, std::string src);

        unsigned int getId() const;
    };
}}}

#endif