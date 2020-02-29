#ifndef SHADERLINK_HPP
#define SHADERLINK_HPP

#include <glad/glad.h>
#include "ShaderProgram.hpp"
#include "Shader.hpp"

class ShaderLink
{
    const ShaderProgram &program;
    const Shader &shader;

public:
    ShaderLink(const ShaderProgram &program, const Shader &shader);
    ShaderLink(const ShaderLink &that) = delete;
    ShaderLink &operator=(const ShaderLink &that) = delete;
    ~ShaderLink();
};

#endif