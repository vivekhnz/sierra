#ifndef GRAPHICS_SHADERPROGRAM_HPP
#define GRAPHICS_SHADERPROGRAM_HPP

#include <glad/glad.h>
#include <vector>
#include "Shader.hpp"

class ShaderProgram
{
    int id;

public:
    ShaderProgram();
    ShaderProgram(const ShaderProgram &that) = delete;
    ShaderProgram &operator=(const ShaderProgram &that) = delete;
    ShaderProgram(ShaderProgram &&) = delete;
    ShaderProgram &operator=(ShaderProgram &&) = delete;

    int getId() const;
    void link(const std::vector<Shader> &shaders);
    void use();

    ~ShaderProgram();
};

#endif