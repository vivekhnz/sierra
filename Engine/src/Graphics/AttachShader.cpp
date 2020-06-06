#include "AttachShader.hpp"

#include <iostream>

namespace Terrain { namespace Engine { namespace Graphics {
    AttachShader::AttachShader(const ShaderProgram &program, const Shader &shader) :
        programId(program.getId()), shaderId(shader.getId())
    {
        glAttachShader(programId, shaderId);
    }

    AttachShader::AttachShader(AttachShader &&other) :
        programId(other.programId), shaderId(other.shaderId)
    {
        other.programId = NULL;
        other.shaderId = NULL;
    }

    AttachShader::~AttachShader()
    {
        if (programId != NULL && shaderId != NULL)
        {
            glDetachShader(programId, shaderId);
        }
    }
}}}