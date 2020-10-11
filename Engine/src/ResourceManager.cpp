#include "ResourceManager.hpp"

namespace Terrain { namespace Engine {
    ResourceManager::ResourceManager() : meshCount(0)
    {
    }

    int ResourceManager::newMesh()
    {
        return meshCount++;
    }

    Graphics::MeshData &ResourceManager::getMesh(int handle)
    {
        return meshes[handle];
    }

    ResourceManager::~ResourceManager()
    {
    }
}}