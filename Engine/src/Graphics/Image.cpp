#include "Image.hpp"

#include <stb/stb_image.h>
#include "../IO/Path.hpp"

Image::Image(std::string path, bool is16Bit)
{
    auto absolutePath = Path::getAbsolutePath(path);
    data = is16Bit ? (void *)stbi_load_16(absolutePath.c_str(), &width, &height, &channels, 0)
                   : (void *)stbi_load(absolutePath.c_str(), &width, &height, &channels, 0);
}

int Image::getWidth() const
{
    return width;
}
int Image::getHeight() const
{
    return height;
}
void *Image::getData() const
{
    return data;
}

unsigned char Image::getValue8(int x, int y, int channel) const
{
    return static_cast<unsigned char *>(data)[(((y * width) + x) * channels) + channel];
}
unsigned short Image::getValue16(int x, int y, int channel) const
{
    return static_cast<unsigned short *>(data)[(((y * width) + x) * channels) + channel];
}

Image::~Image()
{
    stbi_image_free(data);
}