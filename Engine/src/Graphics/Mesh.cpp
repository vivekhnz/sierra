#include "Mesh.hpp"

#include <iostream>
#include "BindVertexArray.hpp"
#include "BindBuffer.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    Mesh::Mesh(GLenum primitiveType, MeshRenderer &renderer) :
        vertexBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
        elementBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW), isInitialized(false),
        renderer(renderer)
    {
        meshData.vertexArrayId = vertexArray.getId();
        meshData.elementCount = 0;
        meshData.primitiveType = primitiveType;
    }

    unsigned int Mesh::getVertexBufferId() const
    {
        return vertexBuffer.getId();
    }

    const MeshData &Mesh::getData() const
    {
        return meshData;
    }

    void Mesh::initialize(
        const std::vector<float> &vertices, const std::vector<unsigned int> &indices)
    {
        // fill buffers
        vertexBuffer.fill(vertices.size() * sizeof(float), vertices.data());
        elementBuffer.fill(indices.size() * sizeof(unsigned int), indices.data());

        // configure VAO
        {
            BindVertexArray bindVa(vertexArray);
            BindBuffer bindVbo(GL_ARRAY_BUFFER, vertexBuffer);
            bindVa.bindElementBuffer(elementBuffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(
                1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
        }

        meshData.elementCount = indices.size();
        isInitialized = true;
    }

    void Mesh::setVertices(const std::vector<float> &vertices)
    {
        vertexBuffer.fill(vertices.size() * sizeof(float), vertices.data());
    }

    void Mesh::draw()
    {
        if (!isInitialized)
        {
            throw std::runtime_error("Mesh not initialized.");
        }
        renderer.renderMesh(meshData);
    }

    Mesh::~Mesh()
    {
    }
}}}