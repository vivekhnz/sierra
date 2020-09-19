#include "MeshRendererComponentManager.hpp"

#include "Material.hpp"
#include "MeshData.hpp"
#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    MeshRendererComponentManager::MeshRendererComponentManager(ResourceManager &resourceMgr) :
        resourceMgr(resourceMgr)
    {
    }

    int MeshRendererComponentManager::create(int entityId, int meshHandle, int materialHandle)
    {
        data.entityId.push_back(entityId);
        data.meshHandle.push_back(meshHandle);
        data.materialHandle.push_back(materialHandle);
        return data.count++;
    }

    void MeshRendererComponentManager::renderMeshes()
    {
        for (int i = 0; i < data.count; i++)
        {
            int &meshHandle = data.meshHandle[i];
            int &materialHandle = data.materialHandle[i];

            Material &material = resourceMgr.getMaterial(materialHandle);
            glUseProgram(material.shaderProgramId);
            glPolygonMode(GL_FRONT_AND_BACK, material.polygonMode);
            for (int i = 0; i < material.textureCount; i++)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, material.textureIds[i]);
            }

            MeshData &meshData = resourceMgr.getMesh(meshHandle);
            glBindVertexArray(meshData.vertexArrayId);
            glDrawElements(meshData.primitiveType, meshData.elementCount, GL_UNSIGNED_INT, 0);
        }
    }

    MeshRendererComponentManager::~MeshRendererComponentManager()
    {
        data.count = 0;
    }
}}}