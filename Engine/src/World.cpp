#include "World.hpp"

namespace Terrain { namespace Engine {
    World::World() : meshCount(0), materialCount(0), meshInstanceCount(0)
    {
    }

    int World::newMesh()
    {
        return meshCount++;
    }

    Graphics::MeshData &World::getMesh(int handle)
    {
        return meshes[handle];
    }

    int World::newMaterial()
    {
        return materialCount++;
    }

    Graphics::Material &World::getMaterial(int handle)
    {
        return materials[handle];
    }

    int World::newMeshInstance()
    {
        return meshInstanceCount++;
    }

    int World::getMeshInstanceCount() const
    {
        return meshInstanceCount;
    }

    Graphics::MeshInstance &World::getMeshInstance(int handle)
    {
        return meshInstances[handle];
    }

    World::~World()
    {
    }
}}