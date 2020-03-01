#ifndef ATTACHSHADER_HPP
#define ATTACHSHADER_HPP

#include <glad/glad.h>
#include "ShaderProgram.hpp"
#include "Shader.hpp"

class AttachShader
{
    int programId;
    int shaderId;

public:
    AttachShader(const ShaderProgram &program, const Shader &shader);
    AttachShader(const AttachShader &that) = delete;
    AttachShader &operator=(const AttachShader &that) = delete;
    AttachShader(AttachShader &&other);
    AttachShader &operator=(AttachShader &&) = delete;
    ~AttachShader();
};

#endif