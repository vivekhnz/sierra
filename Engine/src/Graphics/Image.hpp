#ifndef GRAPHICS_IMAGE_HPP
#define GRAPHICS_IMAGE_HPP

#include "../Common.hpp"
#include <string>

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Image
    {
        void *data;
        int width;
        int height;

    public:
        Image(std::string path, bool is16Bit);
        Image(const Image &that) = delete;
        Image &operator=(const Image &that) = delete;
        Image(Image &&other) = delete;
        Image &operator=(Image &&other) = delete;

        int getWidth() const;
        int getHeight() const;
        void *getData() const;

        ~Image();
    };
}}}

#endif