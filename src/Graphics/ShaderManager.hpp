#ifndef GRAPHICS_SHADERMANAGER_HPP
#define GRAPHICS_SHADERMANAGER_HPP

#include "Shader.hpp"

class ShaderManager
{
    Shader loadFromFile(GLenum shaderType, const char *filePath);

public:
    ShaderManager();
    ShaderManager(const ShaderManager &that) = delete;
    ShaderManager &operator=(const ShaderManager &that) = delete;
    ShaderManager(ShaderManager &&) = delete;
    ShaderManager &operator=(ShaderManager &&) = delete;

    Shader loadVertexShaderFromFile(const char *filePath);
    Shader loadFragmentShaderFromFile(const char *filePath);

    ~ShaderManager();
};

#endif