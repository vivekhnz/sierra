#pragma once

#include "../ViewportContext.hpp"
#include "../EditorState.hpp"
#include "ViewportWorld.hpp"
#include "SceneWorld.hpp"
#include "HeightmapCompositionWorld.hpp"
#include "HeightmapPreviewWorld.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class EditorWorlds
    {
        EngineContext *ctx;
        SceneWorld sceneWorld;
        HeightmapCompositionWorld heightmapCompositionWorld;
        HeightmapPreviewWorld heightmapPreviewWorld;

    public:
        EditorWorlds(EngineContext &ctx);

        void initialize();
        void linkViewport(ViewportWorld viewportWorld, ViewportContext &vctx);
        void update(float deltaTime, const EditorState &state, EditorState &newState);
        void render(ViewportContext &vctx);
    };
}}}}