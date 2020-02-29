#ifndef BINDBUFFER_HPP
#define BINDBUFFER_HPP

#include <glad/glad.h>

class BindBuffer
{
    GLenum bufferType;

public:
    BindBuffer(GLenum bufferType, unsigned int bufferId);
    BindBuffer(const BindBuffer &that) = delete;
    BindBuffer &operator=(const BindBuffer &that) = delete;
    BindBuffer(BindBuffer &&) = delete;
    BindBuffer &operator=(BindBuffer &&) = delete;
    ~BindBuffer();
};

#endif