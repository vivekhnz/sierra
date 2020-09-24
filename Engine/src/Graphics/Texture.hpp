#ifndef GRAPHICS_TEXTURE_HPP
#define GRAPHICS_TEXTURE_HPP

#include "../Common.hpp"
#include "Image.hpp"
#include "Renderer.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Texture
    {
        Graphics::Renderer &renderer;
        unsigned int id;

        int internalFormat;
        int format;
        int type;

        int width;
        int height;

    public:
        Texture(Graphics::Renderer &renderer,
            int width,
            int height,
            int internalFormat,
            int format,
            int type,
            int wrapMode,
            int filterMode);
        Texture(const Texture &that) = delete;
        Texture &operator=(const Texture &that) = delete;
        Texture(Texture &&other) = delete;
        Texture &operator=(Texture &&other) = delete;

        unsigned int getId() const;
        int getWidth() const;
        int getHeight() const;

        void load(const void *pixels);

        ~Texture();
    };
}}}

#endif