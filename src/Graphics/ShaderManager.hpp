#ifndef GRAPHICS_SHADERMANAGER_HPP
#define GRAPHICS_SHADERMANAGER_HPP

#include "Shader.hpp"

class ShaderManager
{
    Shader loadFromFile(GLenum shaderType, std::string filePath);

public:
    ShaderManager();
    ShaderManager(const ShaderManager &that) = delete;
    ShaderManager &operator=(const ShaderManager &that) = delete;
    ShaderManager(ShaderManager &&) = delete;
    ShaderManager &operator=(ShaderManager &&) = delete;

    Shader loadVertexShaderFromFile(std::string filePath);
    Shader loadTessControlShaderFromFile(std::string filePath);
    Shader loadTessEvalShaderFromFile(std::string filePath);
    Shader loadFragmentShaderFromFile(std::string filePath);
    Shader loadComputeShaderFromFile(std::string filePath);

    ~ShaderManager();
};

#endif