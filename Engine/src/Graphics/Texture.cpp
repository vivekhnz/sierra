#include "Texture.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    Texture::Texture(Graphics::Renderer &renderer,
        int width,
        int height,
        int internalFormat,
        int format,
        int type,
        int wrapMode,
        int filterMode) :
        renderer(renderer),
        width(width), height(height), internalFormat(internalFormat), format(format),
        type(type)
    {
        id = renderer.createTexture(wrapMode, filterMode);
    }

    unsigned int Texture::getId() const
    {
        return id;
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
        renderer.updateTexture(id, internalFormat, width, height, format, type, pixels);
    }

    Texture::~Texture()
    {
        renderer.destroyTexture(id);
    }
}}}