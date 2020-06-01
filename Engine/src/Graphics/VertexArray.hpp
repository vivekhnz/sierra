#ifndef GRAPHICS_VERTEXARRAY_HPP
#define GRAPHICS_VERTEXARRAY_HPP

#include <glad/glad.h>

class VertexArray
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