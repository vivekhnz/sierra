#ifndef GRAPHICS_BINDBUFFER_HPP
#define GRAPHICS_BINDBUFFER_HPP

#include "..\Common.hpp"
#include <glad/glad.h>
#include "Buffer.hpp"

class EXPORT BindBuffer
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