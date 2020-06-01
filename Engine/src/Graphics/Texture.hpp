#ifndef GRAPHICS_TEXTURE_HPP
#define GRAPHICS_TEXTURE_HPP

#include "Image.hpp"

class Texture
{
    unsigned int id;

public:
    Texture();
    Texture(const Texture &that) = delete;
    Texture &operator=(const Texture &that) = delete;
    Texture(Texture &&other) = delete;
    Texture &operator=(Texture &&other) = delete;

    void initialize(const Image &image,
        int internalFormat,
        int format,
        int type,
        int wrapMode,
        int filterMode);
    void bind(int slot);

    ~Texture();
};

#endif