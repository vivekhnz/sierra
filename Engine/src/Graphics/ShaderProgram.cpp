#include "ShaderProgram.hpp"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace Terrain { namespace Engine { namespace Graphics {
    ShaderProgram::ShaderProgram(Renderer &renderer) : renderer(renderer)
    {
        handle = renderer.createShaderProgram();
    }

    int ShaderProgram::getId() const
    {
        return renderer.getShaderProgramId(handle);
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

    void ShaderProgram::setMat4(std::string uniformName, bool transpose, glm::mat4 matrix)
    {
        renderer.setShaderProgramUniformMat4(handle, uniformName, transpose, matrix);
    }

    void ShaderProgram::setFloat(std::string uniformName, float value)
    {
        renderer.setShaderProgramUniformFloat(handle, uniformName, value);
    }

    void ShaderProgram::setInt(std::string uniformName, int value)
    {
        renderer.setShaderProgramUniformInt(handle, uniformName, value);
    }

    void ShaderProgram::setBool(std::string uniformName, bool value)
    {
        renderer.setShaderProgramUniformInt(handle, uniformName, value ? 1 : 0);
    }

    void ShaderProgram::setVector2(std::string uniformName, glm::vec2 value)
    {
        renderer.setShaderProgramUniformVector2(handle, uniformName, value);
    }

    void ShaderProgram::setVector3(std::string uniformName, glm::vec3 value)
    {
        renderer.setShaderProgramUniformVector3(handle, uniformName, value);
    }
}}}