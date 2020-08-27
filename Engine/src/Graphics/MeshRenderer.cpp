#include "MeshRenderer.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    MeshRenderer::MeshRenderer() : meshCount(0)
    {
    }

    void MeshRenderer::renderMesh(const MeshInstance &mesh)
    {
        auto meshData = getMesh(mesh.meshHandle);
        glBindVertexArray(meshData.vertexArrayId);
        glDrawElements(meshData.primitiveType, meshData.elementCount, GL_UNSIGNED_INT, 0);
    }

    int MeshRenderer::newMesh()
    {
        return meshCount++;
    }

    MeshData &MeshRenderer::getMesh(int handle)
    {
        return meshes[handle];
    }

    MeshRenderer::~MeshRenderer()
    {
    }
}}}