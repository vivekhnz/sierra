#include "Buffer.hpp"

#include "BindBuffer.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    Buffer::Buffer(GLenum type, GLenum usage) : type(type), usage(usage)
    {
        glGenBuffers(1, &id);
    }

    Buffer::Buffer(Buffer &&other) : id(other.id), type(other.type), usage(other.usage)
    {
        other.id = 0;
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
        if (id != 0)
        {
            glDeleteBuffers(1, &id);
        }
    }
}}}