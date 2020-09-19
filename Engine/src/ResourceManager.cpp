#include "ResourceManager.hpp"

namespace Terrain { namespace Engine {
    ResourceManager::ResourceManager() : meshCount(0), materialCount(0)
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

    int ResourceManager::newMaterial()
    {
        return materialCount++;
    }

    Graphics::Material &ResourceManager::getMaterial(int handle)
    {
        return materials[handle];
    }

    ResourceManager::~ResourceManager()
    {
    }
}}