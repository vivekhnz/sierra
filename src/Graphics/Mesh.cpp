#include "Mesh.hpp"

#include <iostream>
#include "BindVertexArray.hpp"
#include "BindBuffer.hpp"

Mesh::Mesh()
    : vertexBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW),
      elementBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW),
      isInitialized(false)
{
}

void Mesh::initialize(const std::vector<float> &vertices, const std::vector<unsigned int> &indices)
{
    // fill buffers
    vertexBuffer.fill(vertices.size() * sizeof(float), vertices.data());
    elementBuffer.fill(indices.size() * sizeof(unsigned int), indices.data());

    // configure VAO
    {
        BindVertexArray bindVa(vertexArray);
        BindBuffer bindVbo(GL_ARRAY_BUFFER, vertexBuffer);
        bindVa.bindElementBuffer(elementBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
    }

    elementCount = indices.size();
    isInitialized = true;
}

void Mesh::draw()
{
    if (!isInitialized)
    {
        throw std::runtime_error("Mesh not initialized.");
    }
    BindVertexArray bindVa(vertexArray);
    glDrawElements(GL_TRIANGLE_STRIP, elementCount, GL_UNSIGNED_INT, 0);
}

Mesh::~Mesh()
{
}