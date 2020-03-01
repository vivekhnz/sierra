#ifndef BINDBUFFER_HPP
#define BINDBUFFER_HPP

#include <glad/glad.h>
#include "Buffer.hpp"

class BindBuffer
{
    GLenum bufferType;

public:
    BindBuffer(GLenum bufferType, const Buffer &buffer);
    BindBuffer(const BindBuffer &that) = delete;
    BindBuffer &operator=(const BindBuffer &that) = delete;
    BindBuffer(BindBuffer &&) = delete;
    BindBuffer &operator=(BindBuffer &&) = delete;
    ~BindBuffer();
};

#endif