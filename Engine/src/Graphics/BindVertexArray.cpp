#include "BindVertexArray.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    BindVertexArray::BindVertexArray(const VertexArray &vertexArray)
    {
        glBindVertexArray(vertexArray.getId());
    }

    void BindVertexArray::bindElementBuffer(const Buffer &elementBuffer)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer.getId());
    }

    BindVertexArray::~BindVertexArray()
    {
        glBindVertexArray(0);
    }
}}}