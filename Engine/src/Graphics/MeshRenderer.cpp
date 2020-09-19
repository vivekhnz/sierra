#include "MeshRenderer.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    MeshRenderer::MeshRenderer(World &world) : world(world)
    {
    }

    void MeshRenderer::renderMeshes()
    {
        int meshInstanceCount = world.componentManagers.meshRenderer.getMeshInstanceCount();
        for (int i = 0; i < meshInstanceCount; i++)
        {
            int meshHandle = world.componentManagers.meshRenderer.getMeshHandle(i);
            int materialHandle = world.componentManagers.meshRenderer.getMaterialHandle(i);

            Material &material = world.getMaterial(materialHandle);
            glUseProgram(material.shaderProgramId);
            glPolygonMode(GL_FRONT_AND_BACK, material.polygonMode);
            for (int i = 0; i < material.textureCount; i++)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, material.textureIds[i]);
            }

            MeshData &meshData = world.getMesh(meshHandle);
            glBindVertexArray(meshData.vertexArrayId);
            glDrawElements(meshData.primitiveType, meshData.elementCount, GL_UNSIGNED_INT, 0);
        }
    }

    MeshRenderer::~MeshRenderer()
    {
    }
}}}