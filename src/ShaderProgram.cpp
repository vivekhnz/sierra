#include "ShaderProgram.hpp"

#include <iostream>

ShaderProgram::ShaderProgram()
{
    id = glCreateProgram();
}

int ShaderProgram::getId() const
{
    return id;
}

void ShaderProgram::link()
{
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

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(id);
}