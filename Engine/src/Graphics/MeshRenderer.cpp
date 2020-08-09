#include "MeshRenderer.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    MeshRenderer::MeshRenderer()
    {
    }

    void MeshRenderer::renderMesh(const MeshData &mesh)
    {
        glBindVertexArray(mesh.vertexArrayId);
        glDrawElements(mesh.primitiveType, mesh.elementCount, GL_UNSIGNED_INT, 0);
    }

    MeshRenderer::~MeshRenderer()
    {
    }
}}}