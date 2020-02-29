#include "VertexArray.hpp"

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &id);
}

unsigned int VertexArray::getId() const
{
    return id;
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &id);
}