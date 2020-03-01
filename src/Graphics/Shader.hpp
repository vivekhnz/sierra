#ifndef GRAPHICS_SHADER_HPP
#define GRAPHICS_SHADER_HPP

#include <glad/glad.h>
#include <string>

class Shader
{
    int id;

public:
    Shader(GLenum shaderType, std::string src);
    Shader(const Shader &that) = delete;
    Shader &operator=(const Shader &that) = delete;
    Shader(Shader &&other);
    Shader &operator=(Shader &&other) = delete;

    int getId() const;
    void compile();

    ~Shader();
};

#endif