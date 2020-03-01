#include "ShaderManager.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

ShaderManager::ShaderManager()
{
}

Shader ShaderManager::loadFromFile(GLenum shaderType, const char *filePath)
{
    // read shader source
    std::ifstream fileStream;
    fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fileStream.open(filePath);
    std::stringstream stringStream;
    stringStream << fileStream.rdbuf();
    fileStream.close();
    std::string src = stringStream.str();

    // compile shader
    Shader shader(shaderType, src.c_str());
    shader.compile();
    return shader;
}

Shader ShaderManager::loadVertexShaderFromFile(const char *filePath)
{
    return loadFromFile(GL_VERTEX_SHADER, filePath);
}

Shader ShaderManager::loadFragmentShaderFromFile(const char *filePath)
{
    return loadFromFile(GL_FRAGMENT_SHADER, filePath);
}

ShaderManager::~ShaderManager()
{
}