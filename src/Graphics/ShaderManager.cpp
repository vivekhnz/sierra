#include "ShaderManager.hpp"

ShaderManager::ShaderManager()
{
}

Shader ShaderManager::loadFromSource(GLenum shaderType, const char *source)
{
    Shader shader(shaderType, source);
    shader.compile();
    return shader;
}

Shader ShaderManager::loadVertexShaderFromSource(const char *source)
{
    return loadFromSource(GL_VERTEX_SHADER, source);
}

Shader ShaderManager::loadFragmentShaderFromSource(const char *source)
{
    return loadFromSource(GL_FRAGMENT_SHADER, source);
}

ShaderManager::~ShaderManager()
{
}