#ifndef GRAPHICS_GRAPHICSASSETMANAGER_HPP
#define GRAPHICS_GRAPHICSASSETMANAGER_HPP

#include "../Common.hpp"

#include "Renderer.hpp"
#include "../Resources/MaterialResource.hpp"
#include "VertexBufferDescription.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT GraphicsAssetManager
    {
    private:
        struct Materials
        {
            int count;
            std::vector<int> shaderProgramHandle;
            std::vector<int> polygonMode;

            std::vector<int> blendEquation;
            std::vector<int> blendSrcFactor;
            std::vector<int> blendDstFactor;

            std::vector<int> firstTextureIndex;
            std::vector<int> textureCount;
            std::vector<int> textureHandles;

            std::vector<int> firstUniformIndex;
            std::vector<int> uniformCount;
            std::vector<const char *> uniformNames;
            std::vector<UniformValue> uniformValues;

            std::map<int, int> resourceIdToHandle;

            Materials() : count(0)
            {
            }
        } materials;

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
        GraphicsAssetManager(const GraphicsAssetManager &that) = delete;
        GraphicsAssetManager &operator=(const GraphicsAssetManager &that) = delete;
        GraphicsAssetManager(GraphicsAssetManager &&) = delete;
        GraphicsAssetManager &operator=(GraphicsAssetManager &&) = delete;

        void onMaterialsLoaded(const int count, Resources::MaterialResource *resources);
        int &getMaterialShaderProgramHandle(int handle);
        void useMaterial(int handle);
        int &lookupMaterial(int resourceId)
        {
            return materials.resourceIdToHandle[resourceId];
        }

        int createMesh(unsigned int primitiveType,
            const std::vector<Graphics::VertexBufferDescription> &vertexBuffers,
            const std::vector<unsigned int> &indices);
        int &getMeshVertexBufferHandle(int handle, int idx);
        int &getMeshVertexArrayHandle(int handle);
        int &getMeshElementCount(int handle);
        unsigned int &getMeshPrimitiveType(int handle);

        ~GraphicsAssetManager();
    };
}}}

#endif