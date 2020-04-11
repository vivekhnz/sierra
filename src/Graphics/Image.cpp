#include "Image.hpp"

#include <stb/stb_image.h>

Image::Image(std::string path)
{
    data = stbi_load(path.c_str(), &width, &height, &channelCount, 0);
}

int Image::getWidth()
{
    return width;
}

int Image::getHeight()
{
    return height;
}

unsigned char Image::getValue(int x, int y, int channel)
{
    return data[(((y * width) + x) * channelCount) + channel];
}

Image::~Image()
{
    stbi_image_free(data);
}