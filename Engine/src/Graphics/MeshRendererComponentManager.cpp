#include "MeshRendererComponentManager.hpp"

#include "Material.hpp"
#include "MeshData.hpp"
#include <iterator>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Terrain { namespace Engine { namespace Graphics {
    MeshRendererComponentManager::MeshRendererComponentManager(
        ResourceManager &resourceMgr, Graphics::Renderer &renderer) :
        resourceMgr(resourceMgr),
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
        data.firstUniformIndex.push_back(data.uniformLocations.size());
        data.uniformCount.push_back(uniformCount);
        data.uniformNames.insert(
            data.uniformNames.end(), uniformNames.begin(), uniformNames.end());
        data.uniformValues.insert(
            data.uniformValues.end(), uniformValues.begin(), uniformValues.end());

        // calculate uniform locations
        int shaderProgramId = resourceMgr.getMaterial(materialHandle).shaderProgramId;
        for (int i = 0; i < uniformCount; i++)
        {
            data.uniformLocations.push_back(
                glGetUniformLocation(shaderProgramId, uniformNames[i].c_str()));
        }

        entityIdToInstanceId[entityId] = data.count;
        return data.count++;
    }

    void MeshRendererComponentManager::renderMeshes()
    {
        for (int i = 0; i < data.count; i++)
        {
            int &materialHandle = data.materialHandle[i];

            // bind material data
            Material &material = resourceMgr.getMaterial(materialHandle);
            glUseProgram(material.shaderProgramId);
            glPolygonMode(GL_FRONT_AND_BACK, material.polygonMode);
            renderer.bindTextures(material.textureHandles, material.textureCount);

            // bind mesh data
            int &meshHandle = data.meshHandle[i];
            MeshData &meshData = resourceMgr.getMesh(meshHandle);
            glBindVertexArray(meshData.vertexArrayId);

            // set per-instance material uniforms
            int &uniformStart = data.firstUniformIndex[i];
            int &uniformCount = data.uniformCount[i];
            for (int u = 0; u < uniformCount; u++)
            {
                int idx = uniformStart + u;
                unsigned int &loc = data.uniformLocations[idx];
                UniformValue &val = data.uniformValues[idx];
                switch (val.type)
                {
                case UniformType::Float:
                    glProgramUniform1f(material.shaderProgramId, loc, val.f);
                    break;
                case UniformType::Vector2:
                    glProgramUniform2fv(
                        material.shaderProgramId, loc, 1, glm::value_ptr(val.vec2));
                    break;
                }
            }

            // draw mesh instance
            glDrawElements(meshData.primitiveType, meshData.elementCount, GL_UNSIGNED_INT, 0);
        }
    }

    void MeshRendererComponentManager::setMaterialHandle(int i, int materialHandle)
    {
        int &uniformCount = data.uniformCount[i];
        int &firstUniformIndex = data.firstUniformIndex[i];

        // recalculate uniform locations
        int shaderProgramId = resourceMgr.getMaterial(materialHandle).shaderProgramId;
        for (int u = 0; u < uniformCount; u++)
        {
            int idx = firstUniformIndex + u;
            data.uniformLocations[idx] =
                glGetUniformLocation(shaderProgramId, data.uniformNames[idx].c_str());
        }

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