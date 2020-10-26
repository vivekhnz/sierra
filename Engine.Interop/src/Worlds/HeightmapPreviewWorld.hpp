#pragma once

#include "../../../Engine/src/World.hpp"
#include "../ViewportContext.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class HeightmapPreviewWorld
    {
        EngineContext &ctx;
        Engine::World world;

    public:
        HeightmapPreviewWorld(EngineContext &ctx);

        void initialize();
        void linkViewport(ViewportContext &vctx);
        void update(float deltaTime);
        void render(EngineViewContext &vctx);

        int createQuadMaterial();
    };
}}}}