#pragma once

#include "../../Engine/World.hpp"
#include "../EditorState.hpp"
#include "../ViewportContext.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class HeightmapPreviewWorld
    {
        EngineContext &ctx;
        Engine::World world;

    public:
        HeightmapPreviewWorld(EngineContext &ctx);

        void initialize(int heightmapTextureHandle);
        void linkViewport(ViewportContext &vctx);
        void update(float deltaTime, const EditorState &state, EditorState &newState);
        void render(EngineViewContext &vctx);

        int createQuadMaterial(int textureHandle);
    };
}}}}