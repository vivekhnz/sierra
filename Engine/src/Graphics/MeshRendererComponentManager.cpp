#include "MeshRendererComponentManager.hpp"

#include <iterator>
#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    MeshRendererComponentManager::MeshRendererComponentManager(
        GraphicsAssetManager &graphicsAssets, Renderer &renderer) :
        graphicsAssets(graphicsAssets),
        renderer(renderer)
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
            int &shaderProgramHandle =
                graphicsAssets.getMaterialShaderProgramHandle(materialHandle);
            graphicsAssets.useMaterial(materialHandle);

            // bind mesh data
            int &meshHandle = data.meshHandle[i];
            int elementCount = graphicsAssets.getMeshElementCount(meshHandle);
            unsigned int primitiveType = graphicsAssets.getMeshPrimitiveType(meshHandle);
            renderer.bindVertexArray(graphicsAssets.getMeshVertexArrayHandle(meshHandle));

            // set per-instance material uniforms
            renderer.setShaderProgramUniforms(shaderProgramHandle, data.uniformCount[i],
                data.firstUniformIndex[i], data.uniformNames, data.uniformValues);

            // draw mesh instance
            glDrawElements(primitiveType, elementCount, GL_UNSIGNED_INT, 0);
        }
    }

    void MeshRendererComponentManager::setMaterial(int i, int materialResourceId)
    {
        data.materialHandle[i] = graphicsAssets.lookupMaterial(materialResourceId);
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

    void MeshRendererComponentManager::setMaterialUniformMatrix4x4(
        int i, std::string uniformName, glm::mat4 value)
    {
        int &startIndex = data.firstUniformIndex[i];
        int &count = data.uniformCount[i];
        for (int u = 0; u < count; u++)
        {
            int idx = startIndex + u;
            if (data.uniformNames[idx] == uniformName)
            {
                data.uniformValues[idx].mat4 = value;
                break;
            }
        }
    }
}}}