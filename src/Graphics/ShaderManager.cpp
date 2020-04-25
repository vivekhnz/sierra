#include "ShaderManager.hpp"

#include "../IO/OpenFile.hpp"

ShaderManager::ShaderManager()
{
}

Shader ShaderManager::loadFromFile(GLenum shaderType, std::string filePath)
{
    // read shader source
    OpenFile openFile(filePath);
    std::string src = openFile.readAllText();

    // compile shader
    Shader shader(shaderType, src);
    shader.compile();
    return shader;
}

Shader ShaderManager::loadVertexShaderFromFile(std::string filePath)
{
    return loadFromFile(GL_VERTEX_SHADER, filePath);
}

Shader ShaderManager::loadTessEvalShaderFromFile(std::string filePath)
{
    return loadFromFile(GL_TESS_EVALUATION_SHADER, filePath);
}

Shader ShaderManager::loadFragmentShaderFromFile(std::string filePath)
{
    return loadFromFile(GL_FRAGMENT_SHADER, filePath);
}

ShaderManager::~ShaderManager()
{
}