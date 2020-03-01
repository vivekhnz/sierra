#include "BindBuffer.hpp"

BindBuffer::BindBuffer(GLenum bufferType, const Buffer &buffer) : bufferType(bufferType)
{
    glBindBuffer(bufferType, buffer.getId());
}

BindBuffer::~BindBuffer()
{
    glBindBuffer(bufferType, 0);
}