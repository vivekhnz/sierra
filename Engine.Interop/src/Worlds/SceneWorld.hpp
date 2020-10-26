#pragma once

#include "../../../Engine/src/World.hpp"
#include "../ViewportContext.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class SceneWorld
    {
        EngineContext &ctx;
        Engine::World world;

    public:
        SceneWorld(EngineContext &ctx);

        void initialize(int heightmapTextureHandle);
        void linkViewport(ViewportContext &vctx);
        void update(float deltaTime);
        void render(EngineViewContext &vctx);
    };
}}}}