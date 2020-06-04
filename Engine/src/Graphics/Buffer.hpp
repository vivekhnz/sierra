#ifndef GRAPHICS_BUFFER_HPP
#define GRAPHICS_BUFFER_HPP

#include "..\Common.hpp"
#include <glad/glad.h>

class EXPORT Buffer
{
    GLenum type;
    GLenum usage;
    unsigned int id;

public:
    Buffer(GLenum type, GLenum usage);
    Buffer(const Buffer &that) = delete;
    Buffer &operator=(const Buffer &that) = delete;
    Buffer(Buffer &&) = delete;
    Buffer &operator=(Buffer &&) = delete;

    unsigned int getId() const;
    void fill(int size, const void *data);

    ~Buffer();
};

#endif