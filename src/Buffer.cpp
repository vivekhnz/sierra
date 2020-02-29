#include "Buffer.hpp"

#include "BindBuffer.hpp"

Buffer::Buffer(GLenum type, GLenum usage) : type(type), usage(usage)
{
    glGenBuffers(1, &id);
}

unsigned int Buffer::getId() const
{
    return id;
}

void Buffer::fill(int size, const void *data)
{
    BindBuffer bind(type, *this);
    glBufferData(type, size, data, usage);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &id);
}