#ifndef GRAPHICS_IMAGE_HPP
#define GRAPHICS_IMAGE_HPP

#include <string>

class Image
{
    int width;
    int height;
    int channelCount;
    unsigned char *data;

public:
    Image(std::string path);
    Image(const Image &that) = delete;
    Image &operator=(const Image &that) = delete;
    Image(Image &&other) = delete;
    Image &operator=(Image &&other) = delete;

    int getWidth();
    int getHeight();
    unsigned char getValue(int x, int y, int channel);

    ~Image();
};

#endif