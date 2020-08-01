#ifndef GRAPHICS_TEXTURE_HPP
#define GRAPHICS_TEXTURE_HPP

#include "../Common.hpp"
#include "Image.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Texture
    {
        unsigned int id;

        int internalFormat;
        int format;
        int type;

        int width;
        int height;

    public:
        Texture(int width,
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

        int getWidth() const;
        int getHeight() const;

        void load(const void *pixels);
        void bind(int slot);

        ~Texture();
    };
}}}

#endif