#include "Texture.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    Texture::Texture(int width,
        int height,
        int internalFormat,
        int format,
        int type,
        int wrapMode,
        int filterMode) :
        width(width),
        height(height), internalFormat(internalFormat), format(format), type(type)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    }

    int Texture::getWidth() const
    {
        return width;
    }

    int Texture::getHeight() const
    {
        return height;
    }

    void Texture::load(const void *pixels)
    {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void Texture::bind(int slot)
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &id);
    }
}}}