#include "Shader.hpp"

#include <iostream>

Shader::Shader(GLenum shaderType, const char *src)
{
    id = glCreateShader(shaderType);
    glShaderSource(id, 1, &src, NULL);
}

int Shader::getId() const
{
    return id;
}

void Shader::compile()
{
    glCompileShader(id);
    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        throw std::runtime_error("Shader compilation failed: " + std::string(infoLog));
    }
}

Shader::~Shader()
{
    glDeleteShader(id);
}