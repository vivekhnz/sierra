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

    int GraphicsAssetManager::createMesh(unsigned int primitiveType,
        const std::vector<float> &vertices,
        const std::vector<unsigned int> &indices)
    {
        // create renderer resources
        int vertexBufferHandle = renderer.createVertexBuffer(GL_STATIC_DRAW);
        int elementBufferHandle = renderer.createElementBuffer(GL_STATIC_DRAW);
        int vertexArrayHandle = renderer.createVertexArray();

        // fill buffers
        renderer.updateVertexBuffer(
            vertexBufferHandle, vertices.size() * sizeof(float), vertices.data());
        renderer.updateElementBuffer(
            elementBufferHandle, indices.size() * sizeof(unsigned int), indices.data());

        // configure VAO
        renderer.bindVertexArray(vertexArrayHandle);
        glBindBuffer(GL_ARRAY_BUFFER, renderer.getVertexBufferId(vertexBufferHandle));
        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER, renderer.getElementBufferId(elementBufferHandle));
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        // create component data
        meshes.vertexBufferHandle.push_back(vertexBufferHandle);
        meshes.vertexArrayHandle.push_back(vertexArrayHandle);
        meshes.elementCount.push_back(indices.size());
        meshes.primitiveType.push_back(primitiveType);
        return meshes.count++;
    }

    int &GraphicsAssetManager::getMeshVertexBufferHandle(int handle)
    {
        return meshes.vertexBufferHandle[handle];
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