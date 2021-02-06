#include "World.hpp"

#include <glad/glad.h>

namespace Terrain { namespace Engine {
    World::World(EngineContext &ctx) : componentManagers(ctx)
    {
        ctx.registerWorld(*this);
    }

    void World::onTextureReloaded(Resources::TextureResourceData &resource)
    {
        componentManagers.terrainCollider.onTextureReloaded(resource);
    }
}}