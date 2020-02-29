#include "ShaderLink.hpp"

#include <iostream>

ShaderLink::ShaderLink(const ShaderProgram &program, const Shader &shader)
    : programId(program.getId()), shaderId(shader.getId())
{
    glAttachShader(programId, shaderId);
}

ShaderLink::ShaderLink(ShaderLink &&other)
    : programId(other.programId), shaderId(other.shaderId)
{
    other.programId = NULL;
    other.shaderId = NULL;
}

ShaderLink::~ShaderLink()
{
    if (programId != NULL && shaderId != NULL)
    {
        glDetachShader(programId, shaderId);
    }
}