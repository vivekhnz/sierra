#ifndef GRAPHICS_IMAGE_HPP
#define GRAPHICS_IMAGE_HPP

#include "../Common.hpp"
#include <string>

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Image
    {
        void *data;

    public:
        Image(std::string path, bool is16Bit);
        Image(const Image &that) = delete;
        Image &operator=(const Image &that) = delete;
        Image(Image &&other) = delete;
        Image &operator=(Image &&other) = delete;

        void *getData() const;

        ~Image();
    };
}}}

#endif