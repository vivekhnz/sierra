#include "Shader.hpp"

#include <iostream>

namespace Terrain { namespace Engine { namespace Graphics {
    Shader::Shader(GLenum shaderType, std::string src)
    {
        id = glCreateShader(shaderType);
        const char *src_c = src.c_str();
        glShaderSource(id, 1, &src_c, NULL);
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
}}}