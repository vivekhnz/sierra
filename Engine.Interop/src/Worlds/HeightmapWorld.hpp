#pragma once

#include "EditorWorld.hpp"
#include "../../../Engine/src/World.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class HeightmapWorld : public EditorWorld
    {
        EngineContext &ctx;
        Engine::World world;

    public:
        HeightmapWorld(EngineContext &ctx);

        void initialize();
        int addCamera(int inputControllerId);
        void update(float deltaTime);
        void render(EngineViewContext &vctx);
    };
}}}}