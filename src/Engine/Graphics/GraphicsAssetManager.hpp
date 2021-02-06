#ifndef GRAPHICS_GRAPHICSASSETMANAGER_HPP
#define GRAPHICS_GRAPHICSASSETMANAGER_HPP

#include "../Common.hpp"

#include "Renderer.hpp"
#include "VertexBufferDescription.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT GraphicsAssetManager
    {
    private:
        struct Meshes
        {
            int count;
            std::vector<int> firstVertexBufferHandle;
            std::vector<int> vertexBufferHandles;
            std::vector<int> vertexArrayHandle;
            std::vector<int> elementCount;
            std::vector<unsigned int> primitiveType;

            Meshes() : count(0)
            {
            }
        } meshes;

        Renderer &renderer;

    public:
        GraphicsAssetManager(Renderer &renderer);

        int createMesh(unsigned int primitiveType,
            const std::vector<Graphics::VertexBufferDescription> &vertexBuffers,
            const std::vector<unsigned int> &indices);
        int &getMeshVertexBufferHandle(int handle, int idx);
        int &getMeshVertexArrayHandle(int handle);
        int &getMeshElementCount(int handle);
        unsigned int &getMeshPrimitiveType(int handle);
    };
}}}

#endif