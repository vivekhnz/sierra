#ifndef GRAPHICS_SHADERMANAGER_HPP
#define GRAPHICS_SHADERMANAGER_HPP

#include "Shader.hpp"

class ShaderManager
{
    Shader loadFromSource(GLenum shaderType, const char *source);

public:
    ShaderManager();
    ShaderManager(const ShaderManager &that) = delete;
    ShaderManager &operator=(const ShaderManager &that) = delete;
    ShaderManager(ShaderManager &&) = delete;
    ShaderManager &operator=(ShaderManager &&) = delete;

    Shader loadVertexShaderFromSource(const char *source);
    Shader loadFragmentShaderFromSource(const char *source);

    ~ShaderManager();
};

#endif