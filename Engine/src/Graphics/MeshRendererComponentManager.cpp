#include "MeshRendererComponentManager.hpp"

#include "Material.hpp"
#include "MeshData.hpp"
#include <iterator>
#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    MeshRendererComponentManager::MeshRendererComponentManager(ResourceManager &resourceMgr,
        GraphicsAssetManager &graphicsAssets,
        Renderer &renderer) :
        resourceMgr(resourceMgr),
        graphicsAssets(graphicsAssets), renderer(renderer)
    {
    }

    int MeshRendererComponentManager::create(int entityId,
        int meshHandle,
        int materialHandle,
        std::vector<std::string> uniformNames,
        std::vector<UniformValue> uniformValues)
    {
        data.entityId.push_back(entityId);
        data.meshHandle.push_back(meshHandle);
        data.materialHandle.push_back(materialHandle);

        int uniformCount = uniformNames.size();
        data.firstUniformIndex.push_back(data.uniformNames.size());
        data.uniformCount.push_back(uniformCount);
        data.uniformNames.insert(
            data.uniformNames.end(), uniformNames.begin(), uniformNames.end());
        data.uniformValues.insert(
            data.uniformValues.end(), uniformValues.begin(), uniformValues.end());

        entityIdToInstanceId[entityId] = data.count;
        return data.count++;
    }

    void MeshRendererComponentManager::renderMeshes()
    {
        for (int i = 0; i < data.count; i++)
        {
            int &materialHandle = data.materialHandle[i];

            // bind material data
            Material &material = graphicsAssets.getMaterial(materialHandle);
            renderer.useShaderProgram(material.shaderProgramHandle);
            glPolygonMode(GL_FRONT_AND_BACK, material.polygonMode);
            renderer.bindTextures(material.textureHandles, material.textureCount);

            // bind mesh data
            int &meshHandle = data.meshHandle[i];
            MeshData &meshData = resourceMgr.getMesh(meshHandle);
            glBindVertexArray(meshData.vertexArrayId);

            // set per-material uniforms
            renderer.setShaderProgramUniforms(material.shaderProgramHandle,
                material.uniformCount, 0, material.uniformNames, material.uniformValues);

            // set per-instance material uniforms
            renderer.setShaderProgramUniforms(material.shaderProgramHandle,
                data.uniformCount[i], data.firstUniformIndex[i], data.uniformNames,
                data.uniformValues);

            // draw mesh instance
            glDrawElements(meshData.primitiveType, meshData.elementCount, GL_UNSIGNED_INT, 0);
        }
    }

    void MeshRendererComponentManager::setMaterialHandle(int i, int materialHandle)
    {
        data.materialHandle[i] = materialHandle;
    }

    void MeshRendererComponentManager::setMaterialUniformFloat(
        int i, std::string uniformName, float value)
    {
        int &startIndex = data.firstUniformIndex[i];
        int &count = data.uniformCount[i];
        for (int u = 0; u < count; u++)
        {
            int idx = startIndex + u;
            if (data.uniformNames[idx] == uniformName)
            {
                data.uniformValues[idx].f = value;
                break;
            }
        }
    }

    void MeshRendererComponentManager::setMaterialUniformVector2(
        int i, std::string uniformName, glm::vec2 value)
    {
        int &startIndex = data.firstUniformIndex[i];
        int &count = data.uniformCount[i];
        for (int u = 0; u < count; u++)
        {
            int idx = startIndex + u;
            if (data.uniformNames[idx] == uniformName)
            {
                data.uniformValues[idx].vec2 = value;
                break;
            }
        }
    }

    MeshRendererComponentManager::~MeshRendererComponentManager()
    {
        data.count = 0;
    }
}}}