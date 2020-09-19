#include "World.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine {
    World::World(EngineContext &ctx) : componentManagers(ctx)
    {
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