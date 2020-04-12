#include "Texture.hpp"

#include <iostream>
#include <glad/glad.h>

Texture::Texture(int wrapMode, int filterMode)
    : id(NULL), wrapMode(wrapMode), filterMode(filterMode)
{
}

unsigned int Texture::getId() const
{
    return id;
}

void Texture::initialize(Image &image)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.getWidth(), image.getHeight(), 0, GL_RGB,
                 GL_UNSIGNED_BYTE, image.getData());
    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::~Texture()
{
    if (id != NULL)
    {
        glDeleteTextures(1, &id);
    }
}