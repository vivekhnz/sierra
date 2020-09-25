#include "MeshRendererComponentManager.hpp"

#include "Material.hpp"
#include "MeshData.hpp"
#include <iterator>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <map>

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

        int shaderProgramId = resourceMgr.getMaterial(materialHandle).shaderProgramId;

        int uniformCount = uniformNames.size();
        data.firstUniformIndex.push_back(data.uniformLocations.size());
        data.uniformCount.push_back(uniformCount);
        for (int i = 0; i < uniformCount; i++)
        {
            data.uniformLocations.push_back(
                glGetUniformLocation(shaderProgramId, uniformNames[i].c_str()));
        }
        data.uniformValues.insert(
            data.uniformValues.end(), uniformValues.begin(), uniformValues.end());

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
        int &currentMaterialHandle = data.materialHandle[i];

        // once we change material, we will be using a new shader program
        // this means our uniform locations are no longer valid
        // we need to update our uniform locations for the new material's shader program
        std::map<unsigned int, std::string> oldMaterialUniformLocsToUniformNames;

        // build up a map of the current shader program's uniform locations to uniform names
        int shaderProgramId = resourceMgr.getMaterial(currentMaterialHandle).shaderProgramId;
        int uniformCount = 0;
        glGetProgramInterfaceiv(
            shaderProgramId, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformCount);

        GLchar uniformName[256];
        for (int i = 0; i < uniformCount; i++)
        {
            // loop through and retrieve the uniform name
            glGetProgramResourceName(
                shaderProgramId, GL_UNIFORM, i, 256, NULL, &uniformName[0]);

            // get the uniform location from the uniform name
            unsigned loc = glGetUniformLocation(shaderProgramId, uniformName);

            oldMaterialUniformLocsToUniformNames[loc] = uniformName;
        }

        // get the new material's shader program
        shaderProgramId = resourceMgr.getMaterial(materialHandle).shaderProgramId;

        // loop through each material uniform and update its location to point at the
        // respective uniform location for the new material's shader program
        int &materialUniformCount = data.uniformCount[i];
        for (int i = 0; i < materialUniformCount; i++)
        {
            unsigned int &loc = data.uniformLocations[i];
            auto locNamePair = oldMaterialUniformLocsToUniformNames.find(loc);
            if (locNamePair != oldMaterialUniformLocsToUniformNames.end())
            {
                loc = glGetUniformLocation(shaderProgramId, locNamePair->second.c_str());
            }
        }

        // finally, update our material handle to point to the new material
        data.materialHandle[i] = materialHandle;
    }

    MeshRendererComponentManager::~MeshRendererComponentManager()
    {
        data.count = 0;
    }
}}}