#include "ShaderManager.hpp"

#include "../IO/OpenFile.hpp"

ShaderManager::ShaderManager()
{
}

Shader ShaderManager::loadFromFile(GLenum shaderType, std::string filePath) const
{
    // read shader source
    OpenFile openFile(filePath);
    std::string src = openFile.readAllText();

    // compile shader
    Shader shader(shaderType, src);
    shader.compile();
    return shader;
}

Shader ShaderManager::loadVertexShaderFromFile(std::string filePath) const
{
    return loadFromFile(GL_VERTEX_SHADER, filePath);
}
Shader ShaderManager::loadTessControlShaderFromFile(std::string filePath) const
{
    return loadFromFile(GL_TESS_CONTROL_SHADER, filePath);
}
Shader ShaderManager::loadTessEvalShaderFromFile(std::string filePath) const
{
    return loadFromFile(GL_TESS_EVALUATION_SHADER, filePath);
}
Shader ShaderManager::loadFragmentShaderFromFile(std::string filePath) const
{
    return loadFromFile(GL_FRAGMENT_SHADER, filePath);
}
Shader ShaderManager::loadComputeShaderFromFile(std::string filePath) const
{
    return loadFromFile(GL_COMPUTE_SHADER, filePath);
}

ShaderManager::~ShaderManager()
{
}