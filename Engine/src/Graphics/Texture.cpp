#include "Texture.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    Texture::Texture(Graphics::Renderer &renderer,
        int internalFormat,
        int format,
        int type,
        int wrapMode,
        int filterMode) :
        renderer(renderer),
        internalFormat(internalFormat), format(format), type(type)
    {
        handle = renderer.createTexture(wrapMode, filterMode);
    }

    int Texture::getHandle() const
    {
        return handle;
    }

    void Texture::load(int width, int height, const void *pixels)
    {
        renderer.updateTexture(handle, internalFormat, width, height, format, type, pixels);
    }
}}}