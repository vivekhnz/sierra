#include "Image.hpp"

#include <stb/stb_image.h>
#include "../IO/Path.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    Image::Image(std::string path, bool is16Bit)
    {
        data = is16Bit ? (void *)stbi_load_16(path.c_str(), &width, &height, &channels, 0)
                       : (void *)stbi_load(path.c_str(), &width, &height, &channels, 0);
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
}}}