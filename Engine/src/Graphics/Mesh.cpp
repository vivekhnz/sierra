#include "Mesh.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    Mesh::Mesh(Graphics::Renderer &renderer) : renderer(renderer)
    {
        vertexBufferHandle = renderer.createVertexBuffer(GL_STATIC_DRAW);
        elementBufferHandle = renderer.createElementBuffer(GL_STATIC_DRAW);
    }

    unsigned int Mesh::getVertexArrayId() const
    {
        return vertexArray.getId();
    }

    int Mesh::getVertexBufferHandle() const
    {
        return vertexBufferHandle;
    }

    void Mesh::initialize(
        const std::vector<float> &vertices, const std::vector<unsigned int> &indices)
    {
        // fill buffers
        renderer.updateVertexBuffer(
            vertexBufferHandle, vertices.size() * sizeof(float), vertices.data());
        renderer.updateElementBuffer(
            elementBufferHandle, indices.size() * sizeof(unsigned int), indices.data());

        // configure VAO
        glBindVertexArray(vertexArray.getId());
        glBindBuffer(GL_ARRAY_BUFFER, renderer.getVertexBufferId(vertexBufferHandle));
        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER, renderer.getElementBufferId(elementBufferHandle));
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }
}}}