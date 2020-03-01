#include "ShaderProgram.hpp"

#include <iostream>
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

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(id);
}