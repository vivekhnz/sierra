#include "ShaderProgram.hpp"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "AttachShader.hpp"

ShaderProgram::ShaderProgram()
{
    id = glCreateProgram();
}

int ShaderProgram::getId() const
{
    return id;
}

void ShaderProgram::link(const std::vector<Shader> &shaders)
{
    std::vector<AttachShader> attachShaders;
    for (auto &&shader : shaders)
    {
        attachShaders.push_back(AttachShader(*this, shader));
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
}

void ShaderProgram::use()
{
    glUseProgram(id);
}

void ShaderProgram::setMat4(std::string uniformName, bool transpose, glm::mat4 matrix)
{
    unsigned int loc = glGetUniformLocation(id, uniformName.c_str());
    glProgramUniformMatrix4fv(id, loc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(matrix));
}

void ShaderProgram::setFloat(std::string uniformName, float value)
{
    unsigned int loc = glGetUniformLocation(id, uniformName.c_str());
    glProgramUniform1f(id, loc, value);
}

void ShaderProgram::setInt(std::string uniformName, int value)
{
    unsigned int loc = glGetUniformLocation(id, uniformName.c_str());
    glProgramUniform1i(id, loc, value);
}

void ShaderProgram::setVector2(std::string uniformName, glm::vec2 value)
{
    unsigned int loc = glGetUniformLocation(id, uniformName.c_str());
    glProgramUniform2fv(id, loc, 1, glm::value_ptr(value));
}

void ShaderProgram::setVector3(std::string uniformName, glm::vec3 value)
{
    unsigned int loc = glGetUniformLocation(id, uniformName.c_str());
    glProgramUniform3fv(id, loc, 1, glm::value_ptr(value));
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(id);
}