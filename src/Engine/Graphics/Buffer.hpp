#ifndef GRAPHICS_BUFFER_HPP
#define GRAPHICS_BUFFER_HPP

#include "../Common.hpp"
#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT Buffer
    {
        GLenum type;
        GLenum usage;
        unsigned int id;

    public:
        Buffer(GLenum type, GLenum usage);
        Buffer(const Buffer &that);
        Buffer &operator=(const Buffer &that) = delete;
        Buffer(Buffer &&);
        Buffer &operator=(Buffer &&) = delete;

        unsigned int getId() const;
        void fill(int size, const void *data);

        ~Buffer();
    };
}}}

#endif