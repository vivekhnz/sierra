#ifndef GRAPHICS_IMAGE_HPP
#define GRAPHICS_IMAGE_HPP

#include <string>

class Image
{
    int width;
    int height;
    int channels;
    unsigned char *data;

public:
    Image(std::string path);
    Image(const Image &that) = delete;
    Image &operator=(const Image &that) = delete;
    Image(Image &&other) = delete;
    Image &operator=(Image &&other) = delete;

    int getWidth() const;
    int getHeight() const;
    unsigned char *getData() const;
    unsigned char getValue(int x, int y, int channel) const;

    ~Image();
};

#endif