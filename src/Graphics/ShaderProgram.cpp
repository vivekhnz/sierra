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
    unsigned int uniformLoc = glGetUniformLocation(id, uniformName.c_str());
    glUniformMatrix4fv(uniformLoc, 1, transpose ? GL_TRUE : GL_FALSE, glm::value_ptr(matrix));
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(id);
}