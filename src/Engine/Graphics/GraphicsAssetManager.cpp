#include "GraphicsAssetManager.hpp"

#include <glad/glad.h>
#include "../terrain_assets.h"
#include "../terrain_renderer.h"

namespace Terrain { namespace Engine { namespace Graphics {
    GraphicsAssetManager::GraphicsAssetManager(Renderer &renderer) : renderer(renderer)
    {
    }

    int GraphicsAssetManager::createMesh(unsigned int primitiveType,
        const std::vector<Graphics::VertexBufferDescription> &vertexBuffers,
        const std::vector<unsigned int> &indices)
    {
        // create element buffer
        int elementBufferHandle =
            rendererCreateBuffer(renderer.memory, RENDERER_ELEMENT_BUFFER, GL_STATIC_DRAW);
        rendererUpdateBuffer(renderer.memory, elementBufferHandle,
            indices.size() * sizeof(unsigned int), (void *)indices.data());

        // create VAO
        int vertexArrayHandle = rendererCreateVertexArray(renderer.memory);
        rendererBindVertexArray(renderer.memory, vertexArrayHandle);
        rendererBindBuffer(renderer.memory, elementBufferHandle);
        meshes.firstVertexBufferHandle.push_back(meshes.vertexBufferHandles.size());

        unsigned int attributeIdx = 0;
        int vertexBufferCount = vertexBuffers.size();
        for (int i = 0; i < vertexBufferCount; i++)
        {
            const VertexBufferDescription &vertexBufferDesc = vertexBuffers[i];

            // create vertex buffer
            int vertexBufferHandle =
                rendererCreateBuffer(renderer.memory, RENDERER_VERTEX_BUFFER, GL_STATIC_DRAW);
            rendererUpdateBuffer(renderer.memory, vertexBufferHandle, vertexBufferDesc.size,
                vertexBufferDesc.data);
            meshes.vertexBufferHandles.push_back(vertexBufferHandle);

            // calculate stride
            int stride = 0;
            for (int j = 0; j < vertexBufferDesc.attributeCount; j++)
            {
                const VertexAttribute &attr = vertexBufferDesc.attributes[j];
                stride += attr.count * attr.typeSize;
            }

            // bind vertex attributes
            rendererBindBuffer(renderer.memory, vertexBufferHandle);
            unsigned int offset = 0;
            for (int j = 0; j < vertexBufferDesc.attributeCount; j++)
            {
                const VertexAttribute &attr = vertexBufferDesc.attributes[j];
                rendererBindVertexAttribute(attributeIdx, attr.type, attr.isNormalized,
                    attr.count, stride, offset, vertexBufferDesc.isPerInstance);
                offset += attr.count * attr.typeSize;
                attributeIdx++;
            }
        }

        rendererUnbindVertexArray();

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