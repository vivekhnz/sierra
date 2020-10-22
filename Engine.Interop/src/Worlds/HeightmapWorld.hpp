#pragma once

#include "../../../Engine/src/World.hpp"
#include "../ViewportContext.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class HeightmapWorld
    {
        EngineContext &ctx;
        Engine::World world;

    public:
        HeightmapWorld(EngineContext &ctx);

        void initialize();
        void addViewport(ViewportContext &vctx, int inputControllerId);
        void update(float deltaTime);
        void render(EngineViewContext &vctx);
    };
}}}}