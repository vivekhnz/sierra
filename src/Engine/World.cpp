#include "World.hpp"

namespace Terrain { namespace Engine {
    World::World(EngineContext &ctx) : componentManagers(ctx)
    {
        ctx.registerWorld(*this);
    }
}}