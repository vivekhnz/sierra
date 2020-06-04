#ifndef GRAPHICS_MESH_HPP
#define GRAPHICS_MESH_HPP

#include "..\Common.hpp"
#include <vector>
#include "Buffer.hpp"
#include "VertexArray.hpp"

class EXPORT Mesh
{
    bool isInitialized;
    Buffer vertexBuffer;
    Buffer elementBuffer;
    VertexArray vertexArray;
    int elementCount;
    GLenum primitiveType;

public:
    Mesh(GLenum primitiveType);
    Mesh(const Mesh &that) = delete;
    Mesh &operator=(const Mesh &that) = delete;
    Mesh(Mesh &&) = delete;
    Mesh &operator=(Mesh &&) = delete;

    unsigned int getVertexBufferId() const;

    void initialize(
        const std::vector<float> &vertices, const std::vector<unsigned int> &indices);
    void draw();

    ~Mesh();
};

#endif