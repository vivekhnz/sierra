#ifndef GRAPHICS_SHADER_HPP
#define GRAPHICS_SHADER_HPP

#include "../Common.hpp"
#include <glad/glad.h>
#include <string>

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Shader
    {
        int id;

    public:
        Shader(GLenum shaderType, std::string src);
        Shader(const Shader &that) = delete;
        Shader &operator=(const Shader &that) = delete;
        Shader(Shader &&other);
        Shader &operator=(Shader &&other) = delete;

        int getId() const;
        void compile();

        ~Shader();
    };
}}}

#endif