#ifndef GRAPHICS_TEXTURE_HPP
#define GRAPHICS_TEXTURE_HPP

#include "Image.hpp"

class Texture
{
    unsigned int id;
    int wrapMode;
    int filterMode;

public:
    Texture(int wrapMode, int filterMode);
    Texture(const Texture &that) = delete;
    Texture &operator=(const Texture &that) = delete;
    Texture(Texture &&other) = delete;
    Texture &operator=(Texture &&other) = delete;

    unsigned int getId() const;
    void initialize(Image &image);

    ~Texture();
};

#endif