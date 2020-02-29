#ifndef SHADERPROGRAM_HPP
#define SHADERPROGRAM_HPP

#include <glad/glad.h>

class ShaderProgram
{
    int id;

public:
    ShaderProgram();
    ShaderProgram(const ShaderProgram &that) = delete;
    ShaderProgram &operator=(const ShaderProgram &that) = delete;
    int getId() const;
    void link();
    ~ShaderProgram();
};

#endif