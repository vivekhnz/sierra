#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>

class Shader
{
    int id;

public:
    Shader(GLenum shaderType, const char *src);
    Shader(const Shader &that) = delete;
    Shader &operator=(const Shader &that) = delete;
    int getId() const;
    void compile();
    ~Shader();
};

#endif