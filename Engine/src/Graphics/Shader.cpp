#include "Shader.hpp"

#include "Renderer.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    Shader::Shader(Renderer &renderer, GLenum shaderType, std::string src) : renderer(renderer)
    {
        id = renderer.createShader(shaderType, src);
    }

    unsigned int Shader::getId() const
    {
        return renderer.getShaderId(id);
    }
}}}