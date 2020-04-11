#include "Image.hpp"

#include <stb/stb_image.h>

Image::Image(std::string path)
{
    data = stbi_load(path.c_str(), &width, &height, &channels, 0);
}

int Image::getWidth() const
{
    return width;
}
int Image::getHeight() const
{
    return height;
}
unsigned char *Image::getData() const
{
    return data;
}

unsigned char Image::getValue(int x, int y, int channel) const
{
    return data[(((y * width) + x) * channels) + channel];
}

Image::~Image()
{
    stbi_image_free(data);
}