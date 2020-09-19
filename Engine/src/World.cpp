#include "World.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine {
    World::World(EngineContext &ctx) :
        meshCount(0), materialCount(0), meshInstanceCount(0), componentManagers(ctx)
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

    void World::update(float deltaTime)
    {
        componentManagers.orbitCamera.calculateCameraStates(deltaTime);
        componentManagers.firstPersonCamera.calculateCameraStates(deltaTime);
    }

    void World::render()
    {
        glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    World::~World()
    {
    }
}}