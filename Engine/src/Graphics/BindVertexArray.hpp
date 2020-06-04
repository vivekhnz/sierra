#ifndef GRAPHICS_BINDVERTEXARRAY_HPP
#define GRAPHICS_BINDVERTEXARRAY_HPP

#include "..\Common.hpp"
#include <glad/glad.h>
#include "VertexArray.hpp"
#include "Buffer.hpp"

class EXPORT BindVertexArray
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