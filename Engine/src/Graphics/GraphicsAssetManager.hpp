#ifndef GRAPHICS_GRAPHICSASSETMANAGER_HPP
#define GRAPHICS_GRAPHICSASSETMANAGER_HPP

#include "../Common.hpp"

#include "Renderer.hpp"
#include "../Resources/MaterialResource.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    class EXPORT GraphicsAssetManager
    {
    private:
        struct Materials
        {
            int count;
            std::vector<int> shaderProgramHandle;
            std::vector<int> polygonMode;

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

        Renderer &renderer;

    public:
        GraphicsAssetManager(Renderer &renderer);

        void onMaterialsLoaded(const int count, Resources::MaterialResource *resources);
        void createMaterial(int resourceId,
            int shaderProgramResourceId,
            int polygonMode,
            std::vector<int> textureResourceIds,
            std::vector<std::string> uniformNames,
            std::vector<UniformValue> uniformValues);
        int &getMaterialShaderProgramHandle(int handle);
        void useMaterial(int handle);
        int lookupMaterial(int resourceId)
        {
            return materials.resourceIdToHandle[resourceId];
        }
    };
}}}

#endif