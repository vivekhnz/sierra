#include "BindVertexArray.hpp"

BindVertexArray::BindVertexArray(const VertexArray &vertexArray)
{
    glBindVertexArray(vertexArray.getId());
}

BindVertexArray::~BindVertexArray()
{
    glBindVertexArray(0);
}