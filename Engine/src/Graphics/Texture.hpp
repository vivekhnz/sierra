#ifndef GRAPHICS_TEXTURE_HPP
#define GRAPHICS_TEXTURE_HPP

#include "../Common.hpp"
#include "Image.hpp"
#include "Renderer.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Texture
    {
        Graphics::Renderer &renderer;
        int handle;

        int internalFormat;
        int format;
        int type;

    public:
        Texture(Graphics::Renderer &renderer,
            int internalFormat,
            int format,
            int type,
            int wrapMode,
            int filterMode);

        int getHandle() const;

        void load(int width, int height, const void *pixels);
    };
}}}

#endif