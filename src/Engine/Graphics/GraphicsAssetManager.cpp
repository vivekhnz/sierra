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

            materials.blendEquation.push_back(resource.blendEquation);
            materials.blendSrcFactor.push_back(resource.blendSrcFactor);
            materials.blendDstFactor.push_back(resource.blendDstFactor);

            materials.firstTextureIndex.push_back(materials.textureHandles.size());
            materials.textureCount.push_back(resource.textureCount);
            for (int t = 0; t < resource.textureCount; t++)
            {
                materials.textureHandles.push_back(
                    renderer.lookupTexture(resource.textureResourceIds[t]));
            }

            materials.firstUniformIndex.push_back(materials.uniformNames.size());
            materials.uniformCount.push_back(resource.uniformCount);

            // resource.uniformNames is a contiguous set of null-terminated strings
            const char *srcStartCursor = resource.uniformNames;
            const char *srcEndCursor = srcStartCursor;
            int u = 0;
            while (u < resource.uniformCount)
            {
                if (!(*srcEndCursor++))
                {
                    int nameLength = srcEndCursor - srcStartCursor;
                    char *uniformName = new char[nameLength];
                    memcpy(uniformName, srcStartCursor, nameLength);

                    materials.uniformNames.push_back(uniformName);
                    materials.uniformValues.push_back(resource.uniformValues[u]);

                    srcStartCursor = srcEndCursor;
                    u++;
                }
            }

            materials.resourceIdToHandle[resource.id] = materials.count++;
        }
    }

    int GraphicsAssetManager::createMaterial(int shaderProgramHandle,
        int polygonMode,
        int blendEquation,
        int blendSrcFactor,
        int blendDstFactor,
        int textureCount,
        int *textureHandles,
        int uniformCount,
        const char *uniformNames,
        Graphics::UniformValue *uniformValues)
    {
        materials.shaderProgramHandle.push_back(shaderProgramHandle);
        materials.polygonMode.push_back(polygonMode);

        materials.blendEquation.push_back(blendEquation);
        materials.blendSrcFactor.push_back(blendSrcFactor);
        materials.blendDstFactor.push_back(blendDstFactor);

        materials.firstTextureIndex.push_back(materials.textureHandles.size());
        materials.textureCount.push_back(textureCount);
        for (int t = 0; t < textureCount; t++)
        {
            materials.textureHandles.push_back(textureHandles[t]);
        }

        materials.firstUniformIndex.push_back(materials.uniformNames.size());
        materials.uniformCount.push_back(uniformCount);

        // uniformNames is a contiguous set of null-terminated strings
        const char *srcStartCursor = uniformNames;
        const char *srcEndCursor = srcStartCursor;
        int u = 0;
        while (u < uniformCount)
        {
            if (!(*srcEndCursor++))
            {
                int nameLength = srcEndCursor - srcStartCursor;
                char *uniformName = new char[nameLength];
                memcpy(uniformName, srcStartCursor, nameLength);

                materials.uniformNames.push_back(uniformName);
                materials.uniformValues.push_back(uniformValues[u]);

                srcStartCursor = srcEndCursor;
                u++;
            }
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
        int firstUniformIndex = materials.firstUniformIndex[handle];

        renderer.useShaderProgram(shaderProgramHandle);
        renderer.setPolygonMode(materials.polygonMode[handle]);
        renderer.setBlendMode(materials.blendEquation[handle],
            materials.blendSrcFactor[handle], materials.blendDstFactor[handle]);

        Graphics::Renderer::ShaderProgramState shaderProgramState = {};
        shaderProgramState.uniforms.count = materials.uniformCount[handle];
        shaderProgramState.uniforms.names = materials.uniformNames.data() + firstUniformIndex;
        shaderProgramState.uniforms.values =
            materials.uniformValues.data() + firstUniformIndex;
        shaderProgramState.textures.count = materials.textureCount[handle];
        shaderProgramState.textures.handles =
            materials.textureHandles.data() + materials.firstTextureIndex[handle];
        renderer.setShaderProgramState(shaderProgramHandle, shaderProgramState);
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

    GraphicsAssetManager::~GraphicsAssetManager()
    {
        for (int i = 0; i < materials.uniformNames.size(); i++)
        {
            delete[] materials.uniformNames[i];
        }
    }
}}}