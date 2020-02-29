#include "BindBuffer.hpp"

BindBuffer::BindBuffer(GLenum bufferType, unsigned int bufferId) : bufferType(bufferType)
{
    glBindBuffer(bufferType, bufferId);
}

BindBuffer::~BindBuffer()
{
    glBindBuffer(bufferType, 0);
}