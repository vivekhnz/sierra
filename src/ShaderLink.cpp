#include "ShaderLink.hpp"

#include <iostream>

ShaderLink::ShaderLink(const ShaderProgram &program, const Shader &shader)
    : program(program), shader(shader)
{
    glAttachShader(program.getId(), shader.getId());
}

ShaderLink::~ShaderLink()
{
    glDetachShader(program.getId(), shader.getId());
}