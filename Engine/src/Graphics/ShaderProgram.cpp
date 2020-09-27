#include "ShaderProgram.hpp"

#include <iostream>

namespace Terrain { namespace Engine { namespace Graphics {
    ShaderProgram::ShaderProgram(Renderer &renderer) : renderer(renderer)
    {
        handle = renderer.createShaderProgram();
    }

    int ShaderProgram::getHandle() const
    {
        return handle;
    }

    void ShaderProgram::link(const std::vector<int> &shaderHandles)
    {
        unsigned int id = renderer.getShaderProgramId(handle);
        for (int shaderHandle : shaderHandles)
        {
            glAttachShader(id, renderer.getShaderId(shaderHandle));
        }

        glLinkProgram(id);
        int success;
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(id, 512, NULL, infoLog);
            throw std::runtime_error("Shader linking failed: " + std::string(infoLog));
        }

        for (int shaderHandle : shaderHandles)
        {
            glDetachShader(id, renderer.getShaderId(shaderHandle));
        }
    }
}}}