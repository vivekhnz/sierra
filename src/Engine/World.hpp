#ifndef WORLD_HPP
#define WORLD_HPP

#include "Common.hpp"
#include "EngineContext.hpp"
#include "Physics/TerrainColliderComponentManager.hpp"
#include "Graphics/MeshRendererComponentManager.hpp"
#include "TerrainRendererComponentManager.hpp"

namespace Terrain { namespace Engine {
    class EXPORT World
    {
    public:
        struct ComponentManagers
        {
            Physics::TerrainColliderComponentManager terrainCollider;
            Graphics::MeshRendererComponentManager meshRenderer;
            TerrainRendererComponentManager terrainRenderer;

            ComponentManagers(EngineContext &ctx) :
                meshRenderer(ctx.assets.graphics, ctx.renderer),
                terrainRenderer(ctx.renderer, meshRenderer, ctx.assets.graphics)
            {
            }
        };
        ComponentManagers componentManagers;

        World(EngineContext &ctx);

        void onTextureReloaded(Resources::TextureResourceData &resource);
    };
}}

#endif