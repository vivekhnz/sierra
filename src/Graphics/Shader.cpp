#include "Shader.hpp"

#include <iostream>

Shader::Shader(GLenum shaderType, const char *src)
{
    id = glCreateShader(shaderType);
    glShaderSource(id, 1, &src, NULL);
}

Shader::Shader(Shader &&other) : id(other.id)
{
    other.id = NULL;
}

int Shader::getId() const
{
    return id;
}

void Shader::compile()
{
    if (id == NULL)
    {
        throw std::runtime_error("Shader not initialized.");
    }

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
    if (id != NULL)
    {
        glDeleteShader(id);
    }
}