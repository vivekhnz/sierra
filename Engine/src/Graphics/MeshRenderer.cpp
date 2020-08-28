#include "MeshRenderer.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    MeshRenderer::MeshRenderer(World &world) : world(world)
    {
    }

    void MeshRenderer::renderMeshes()
    {
        int meshInstanceCount = world.getMeshInstanceCount();
        for (int i = 0; i < meshInstanceCount; i++)
        {
            auto meshInstance = world.getMeshInstance(i);

            auto material = world.getMaterial(meshInstance.materialHandle);
            glUseProgram(material.shaderProgramId);
            glPolygonMode(GL_FRONT_AND_BACK, material.polygonMode);
            for (int i = 0; i < material.textureCount; i++)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, material.textureIds[i]);
            }

            auto meshData = world.getMesh(meshInstance.meshHandle);
            glBindVertexArray(meshData.vertexArrayId);
            glDrawElements(meshData.primitiveType, meshData.elementCount, GL_UNSIGNED_INT, 0);
        }
    }

    MeshRenderer::~MeshRenderer()
    {
    }
}}}