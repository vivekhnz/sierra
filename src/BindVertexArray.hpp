#ifndef BINDVERTEXARRAY_HPP
#define BINDVERTEXARRAY_HPP

#include <glad/glad.h>
#include "VertexArray.hpp"
#include "Buffer.hpp"

class BindVertexArray
{
public:
    BindVertexArray(const VertexArray &vertexArray);
    BindVertexArray(const BindVertexArray &that) = delete;
    BindVertexArray &operator=(const BindVertexArray &that) = delete;
    BindVertexArray(BindVertexArray &&) = delete;
    BindVertexArray &operator=(BindVertexArray &&) = delete;

    void bindElementBuffer(const Buffer &elementBuffer);

    ~BindVertexArray();
};

#endif