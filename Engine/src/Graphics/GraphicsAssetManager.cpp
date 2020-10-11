#include "GraphicsAssetManager.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    GraphicsAssetManager::GraphicsAssetManager(Renderer &renderer) : renderer(renderer)
    {
    }

    int GraphicsAssetManager::createMaterial()
    {
        materials.data.push_back({});
        return materials.count++;
    }

    Material &GraphicsAssetManager::getMaterial(int handle)
    {
        return materials.data[handle];
    }
}}}