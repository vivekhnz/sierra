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
            std::vector<std::string> uniformNames;
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

        void onMaterialsLoaded(const int count, Resources::MaterialResource *resources);
        int createMaterial(int shaderProgramHandle,
            int polygonMode,
            int blendEquation,
            int blendSrcFactor,
            int blendDstFactor,
            int textureCount,
            int *textureHandles,
            int uniformCount,
            int *uniformNameLengths,
            const char *uniformNames,
            Graphics::UniformValue *uniformValues);
        int &getMaterialShaderProgramHandle(int handle);
        void useMaterial(int handle);
        void setMaterialTexture(int materialHandle, int slot, int textureHandle);
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
    };
}}}

#endif