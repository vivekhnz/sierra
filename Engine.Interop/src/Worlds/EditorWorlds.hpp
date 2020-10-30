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
        SceneWorld sceneWorld;
        HeightmapCompositionWorld heightmapCompositionWorld;
        HeightmapPreviewWorld heightmapPreviewWorld;

    public:
        EditorWorlds(EngineContext &ctx);

        void initialize();
        void linkViewport(ViewportWorld viewportWorld, ViewportContext &vctx);
        void update(float deltaTime, EditorState &state);
        void render(ViewportContext &vctx);
    };
}}}}