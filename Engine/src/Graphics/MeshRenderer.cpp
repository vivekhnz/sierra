#include "MeshRenderer.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    MeshRenderer::MeshRenderer(World &world) : world(world)
    {
    }

    void MeshRenderer::renderMesh(int meshInstanceHandle)
    {
        auto meshInstance = world.getMeshInstance(meshInstanceHandle);

        glUseProgram(meshInstance.shaderProgramId);
        glPolygonMode(GL_FRONT_AND_BACK, meshInstance.polygonMode);

        auto meshData = world.getMesh(meshInstance.meshHandle);
        glBindVertexArray(meshData.vertexArrayId);
        glDrawElements(meshData.primitiveType, meshData.elementCount, GL_UNSIGNED_INT, 0);
    }

    MeshRenderer::~MeshRenderer()
    {
    }
}}}