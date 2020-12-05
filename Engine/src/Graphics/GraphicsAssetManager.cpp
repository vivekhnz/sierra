#include "GraphicsAssetManager.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine { namespace Graphics {
    GraphicsAssetManager::GraphicsAssetManager(Renderer &renderer) : renderer(renderer)
    {
    }

    void GraphicsAssetManager::onMaterialsLoaded(
        const int count, Resources::MaterialResource *resources)
    {
        for (int i = 0; i < count; i++)
        {
            Resources::MaterialResource &resource = resources[i];

            materials.shaderProgramHandle.push_back(
                renderer.lookupShaderProgram(resource.shaderProgramResourceId));
            materials.polygonMode.push_back(resource.polygonMode);

            materials.firstTextureIndex.push_back(materials.textureHandles.size());
            materials.textureCount.push_back(resource.textureCount);
            for (int t = 0; t < resource.textureCount; t++)
            {
                materials.textureHandles.push_back(
                    renderer.lookupTexture(resource.textureResourceIds[t]));
            }

            int currentUniformCount = materials.uniformNames.size();
            int newUniformCount = currentUniformCount + resource.uniformCount;
            materials.firstUniformIndex.push_back(currentUniformCount);
            materials.uniformCount.push_back(resource.uniformCount);
            materials.uniformNames.resize(newUniformCount);
            materials.uniformValues.resize(newUniformCount);

            int uniformNameStart = 0;
            for (int u = 0; u < resource.uniformCount; u++)
            {
                int idx = currentUniformCount + u;

                int uniformNameLength = resource.uniformNameLengths[u];
                char *uniformName = new char[uniformNameLength + 1];
                memcpy(
                    uniformName, &resource.uniformNames[uniformNameStart], uniformNameLength);
                uniformName[uniformNameLength] = '\0';
                uniformNameStart += uniformNameLength;

                materials.uniformNames[idx] = uniformName;
                materials.uniformValues[idx] = resource.uniformValues[u];
                delete[] uniformName;
            }

            materials.resourceIdToHandle[resource.id] = materials.count++;
        }
    }

    int GraphicsAssetManager::createMaterial(int shaderProgramHandle,
        int polygonMode,
        int textureCount,
        int *textureHandles,
        int uniformCount,
        int *uniformNameLengths,
        const char *uniformNames,
        Graphics::UniformValue *uniformValues)
    {
        materials.shaderProgramHandle.push_back(shaderProgramHandle);
        materials.polygonMode.push_back(polygonMode);

        materials.firstTextureIndex.push_back(materials.textureHandles.size());
        materials.textureCount.push_back(textureCount);
        for (int t = 0; t < textureCount; t++)
        {
            materials.textureHandles.push_back(textureHandles[t]);
        }

        int currentUniformCount = materials.uniformNames.size();
        int newUniformCount = currentUniformCount + uniformCount;
        materials.firstUniformIndex.push_back(currentUniformCount);
        materials.uniformCount.push_back(uniformCount);
        materials.uniformNames.resize(newUniformCount);
        materials.uniformValues.resize(newUniformCount);

        int uniformNameStart = 0;
        for (int u = 0; u < uniformCount; u++)
        {
            int idx = currentUniformCount + u;

            int uniformNameLength = uniformNameLengths[u];
            char *uniformName = new char[uniformNameLength + 1];
            memcpy(uniformName, &uniformNames[uniformNameStart], uniformNameLength);
            uniformName[uniformNameLength] = '\0';
            uniformNameStart += uniformNameLength;

            materials.uniformNames[idx] = uniformName;
            materials.uniformValues[idx] = uniformValues[u];
            delete[] uniformName;
        }

        return materials.count++;
    }

    int &GraphicsAssetManager::getMaterialShaderProgramHandle(int handle)
    {
        return materials.shaderProgramHandle[handle];
    }

    void GraphicsAssetManager::useMaterial(int handle)
    {
        int &shaderProgramHandle = materials.shaderProgramHandle[handle];

        renderer.useShaderProgram(shaderProgramHandle);
        renderer.setPolygonMode(materials.polygonMode[handle]);
        renderer.bindTextures(
            materials.textureHandles.data() + materials.firstTextureIndex[handle],
            materials.textureCount[handle]);

        renderer.setShaderProgramUniforms(shaderProgramHandle, materials.uniformCount[handle],
            materials.firstUniformIndex[handle], materials.uniformNames,
            materials.uniformValues);
    }

    void GraphicsAssetManager::setMaterialTexture(
        int materialHandle, int slot, int textureHandle)
    {
        int idx = materials.firstTextureIndex[materialHandle] + slot;
        materials.textureHandles[idx] = textureHandle;
    }

    int GraphicsAssetManager::createMesh(unsigned int primitiveType,
        const std::vector<Graphics::VertexBufferDescription> &vertexBuffers,
        const std::vector<unsigned int> &indices)
    {
        // create element buffer
        int elementBufferHandle = renderer.createElementBuffer(GL_STATIC_DRAW);
        renderer.updateElementBuffer(
            elementBufferHandle, indices.size() * sizeof(unsigned int), indices.data());

        // create VAO
        int vertexArrayHandle = renderer.createVertexArray();
        renderer.bindVertexArray(vertexArrayHandle);
        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER, renderer.getElementBufferId(elementBufferHandle));
        meshes.firstVertexBufferHandle.push_back(meshes.vertexBufferHandles.size());

        int attributeIdx = 0;
        int vertexBufferCount = vertexBuffers.size();
        for (int i = 0; i < vertexBufferCount; i++)
        {
            const VertexBufferDescription &vertexBufferDesc = vertexBuffers[i];

            // create vertex buffer
            int vertexBufferHandle = renderer.createVertexBuffer(GL_STATIC_DRAW);
            renderer.updateVertexBuffer(
                vertexBufferHandle, vertexBufferDesc.size, vertexBufferDesc.data);
            meshes.vertexBufferHandles.push_back(vertexBufferHandle);

            // calculate stride
            int stride = 0;
            for (int j = 0; j < vertexBufferDesc.attributeCount; j++)
            {
                const VertexAttribute &attr = vertexBufferDesc.attributes[j];
                stride += attr.count * attr.typeSize;
            }

            // bind vertex attributes
            glBindBuffer(GL_ARRAY_BUFFER, renderer.getVertexBufferId(vertexBufferHandle));
            int offset = 0;
            int attributeDivisor = vertexBufferDesc.isPerInstance ? 1 : 0;
            for (int j = 0; j < vertexBufferDesc.attributeCount; j++)
            {
                const VertexAttribute &attr = vertexBufferDesc.attributes[j];
                glVertexAttribPointer(attributeIdx, attr.count, attr.type, attr.isNormalized,
                    stride, (void *)offset);
                glEnableVertexAttribArray(attributeIdx);
                glVertexAttribDivisor(attributeIdx, attributeDivisor);
                offset += attr.count * attr.typeSize;
                attributeIdx++;
            }
        }

        glBindVertexArray(0);

        // create component data
        meshes.vertexArrayHandle.push_back(vertexArrayHandle);
        meshes.elementCount.push_back(indices.size());
        meshes.primitiveType.push_back(primitiveType);
        return meshes.count++;
    }

    int &GraphicsAssetManager::getMeshVertexBufferHandle(int handle, int idx)
    {
        return meshes.vertexBufferHandles[meshes.firstVertexBufferHandle[handle] + idx];
    }

    int &GraphicsAssetManager::getMeshVertexArrayHandle(int handle)
    {
        return meshes.vertexArrayHandle[handle];
    }

    int &GraphicsAssetManager::getMeshElementCount(int handle)
    {
        return meshes.elementCount[handle];
    }

    unsigned int &GraphicsAssetManager::getMeshPrimitiveType(int handle)
    {
        return meshes.primitiveType[handle];
    }
}}}