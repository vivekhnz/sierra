#ifndef WORLD_HPP
#define WORLD_HPP

#include "Common.hpp"
#include "EngineContext.hpp"
#include "TerrainRendererComponentManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT World
    {
    public:
        struct ComponentManagers
        {
            TerrainRendererComponentManager terrainRenderer;

            ComponentManagers(EngineContext &ctx) : terrainRenderer(ctx.assets.graphics)
            {
            }
        };
        ComponentManagers componentManagers;

        World(EngineContext &ctx);
    };
}}

#endif