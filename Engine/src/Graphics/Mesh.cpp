#include "Mesh.hpp"

#include "BindVertexArray.hpp"
#include "BindBuffer.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    Mesh::Mesh(Graphics::Renderer &renderer) :
        renderer(renderer), elementBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW)
    {
        vertexBufferHandle = renderer.createVertexBuffer(GL_STATIC_DRAW);
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
        elementBuffer.fill(indices.size() * sizeof(unsigned int), indices.data());

        // configure VAO
        {
            BindVertexArray bindVa(vertexArray);
            glBindBuffer(GL_ARRAY_BUFFER, renderer.getVertexBufferId(vertexBufferHandle));
            bindVa.bindElementBuffer(elementBuffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(
                1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
        }
    }

    Mesh::~Mesh()
    {
    }
}}}