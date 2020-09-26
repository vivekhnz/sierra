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
        unsigned int id = renderer.getShaderProgramId(handle);
        unsigned int loc = glGetUniformLocation(id, uniformName.c_str());
        glProgramUniformMatrix4fv(
            id, loc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(matrix));
    }

    void ShaderProgram::setFloat(std::string uniformName, float value)
    {
        unsigned int id = renderer.getShaderProgramId(handle);
        unsigned int loc = glGetUniformLocation(id, uniformName.c_str());
        glProgramUniform1f(id, loc, value);
    }

    void ShaderProgram::setInt(std::string uniformName, int value)
    {
        unsigned int id = renderer.getShaderProgramId(handle);
        unsigned int loc = glGetUniformLocation(id, uniformName.c_str());
        glProgramUniform1i(id, loc, value);
    }

    void ShaderProgram::setBool(std::string uniformName, bool value)
    {
        setInt(uniformName, value ? 1 : 0);
    }

    void ShaderProgram::setVector2(std::string uniformName, glm::vec2 value)
    {
        unsigned int id = renderer.getShaderProgramId(handle);
        unsigned int loc = glGetUniformLocation(id, uniformName.c_str());
        glProgramUniform2fv(id, loc, 1, glm::value_ptr(value));
    }

    void ShaderProgram::setVector3(std::string uniformName, glm::vec3 value)
    {
        unsigned int id = renderer.getShaderProgramId(handle);
        unsigned int loc = glGetUniformLocation(id, uniformName.c_str());
        glProgramUniform3fv(id, loc, 1, glm::value_ptr(value));
    }
}}}