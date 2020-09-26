#include "ShaderManager.hpp"

#include <glad/glad.h>
#include "Renderer.hpp"
#include "../IO/OpenFile.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    ShaderManager::ShaderManager(Renderer &renderer) : renderer(renderer)
    {
    }

    int ShaderManager::loadFromFile(unsigned int shaderType, std::string filePath) const
    {
        // read shader source
        IO::OpenFile openFile(filePath);
        std::string src = openFile.readAllText();

        // compile shader
        return renderer.createShader(shaderType, src);
    }

    int ShaderManager::loadVertexShaderFromFile(std::string filePath) const
    {
        return loadFromFile(GL_VERTEX_SHADER, filePath);
    }
    int ShaderManager::loadTessControlShaderFromFile(std::string filePath) const
    {
        return loadFromFile(GL_TESS_CONTROL_SHADER, filePath);
    }
    int ShaderManager::loadTessEvalShaderFromFile(std::string filePath) const
    {
        return loadFromFile(GL_TESS_EVALUATION_SHADER, filePath);
    }
    int ShaderManager::loadFragmentShaderFromFile(std::string filePath) const
    {
        return loadFromFile(GL_FRAGMENT_SHADER, filePath);
    }
    int ShaderManager::loadComputeShaderFromFile(std::string filePath) const
    {
        return loadFromFile(GL_COMPUTE_SHADER, filePath);
    }

    ShaderManager::~ShaderManager()
    {
    }
}}}