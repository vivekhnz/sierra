#ifndef GRAPHICS_SHADERMANAGER_HPP
#define GRAPHICS_SHADERMANAGER_HPP

#include "../Common.hpp"
#include "Shader.hpp"

class EXPORT ShaderManager
{
    Shader loadFromFile(GLenum shaderType, std::string filePath) const;

public:
    ShaderManager();
    ShaderManager(const ShaderManager &that) = delete;
    ShaderManager &operator=(const ShaderManager &that) = delete;
    ShaderManager(ShaderManager &&) = delete;
    ShaderManager &operator=(ShaderManager &&) = delete;

    Shader loadVertexShaderFromFile(std::string filePath) const;
    Shader loadTessControlShaderFromFile(std::string filePath) const;
    Shader loadTessEvalShaderFromFile(std::string filePath) const;
    Shader loadFragmentShaderFromFile(std::string filePath) const;
    Shader loadComputeShaderFromFile(std::string filePath) const;

    ~ShaderManager();
};

#endif