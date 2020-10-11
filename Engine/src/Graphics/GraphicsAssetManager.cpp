#include "GraphicsAssetManager.hpp"

namespace Terrain { namespace Engine { namespace Graphics {
    GraphicsAssetManager::GraphicsAssetManager(Renderer &renderer) : renderer(renderer)
    {
    }

    int GraphicsAssetManager::createMaterial(int shaderProgramResourceId, int polygonMode)
    {
        materials.shaderProgramHandle.push_back(
            renderer.lookupShaderProgram(shaderProgramResourceId));
        materials.polygonMode.push_back(polygonMode);
        materials.data.push_back({});
        return materials.count++;
    }

    int &GraphicsAssetManager::getMaterialShaderProgramHandle(int handle)
    {
        return materials.shaderProgramHandle[handle];
    }

    int &GraphicsAssetManager::getMaterialPolygonMode(int handle)
    {
        return materials.polygonMode[handle];
    }

    Material &GraphicsAssetManager::getMaterial(int handle)
    {
        return materials.data[handle];
    }
}}}