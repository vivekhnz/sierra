#ifndef GRAPHICS_VERTEXARRAY_HPP
#define GRAPHICS_VERTEXARRAY_HPP

#include "..\Common.hpp"
#include <glad/glad.h>

class EXPORT VertexArray
{
    unsigned int id;

public:
    VertexArray();
    VertexArray(const VertexArray &that) = delete;
    VertexArray &operator=(const VertexArray &that) = delete;
    VertexArray(VertexArray &&) = delete;
    VertexArray &operator=(VertexArray &&) = delete;

    unsigned int getId() const;

    ~VertexArray();
};

#endif