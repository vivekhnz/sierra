#include "Texture.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    Texture::Texture() : id(NULL)
    {
    }

    void Texture::initialize(const Image &image,
        int internalFormat,
        int format,
        int type,
        int wrapMode,
        int filterMode)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.getWidth(), image.getHeight(), 0,
            format, type, image.getData());
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void Texture::bind(int slot)
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    Texture::~Texture()
    {
        if (id != NULL)
        {
            glDeleteTextures(1, &id);
        }
    }
}}}