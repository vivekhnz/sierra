#pragma once

#include "../ViewportContext.hpp"
#include "EditorWorld.hpp"
#include "SceneWorld.hpp"
#include "HeightmapWorld.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class EditorWorlds
    {
        SceneWorld sceneWorld;
        HeightmapWorld heightmapWorld;

    public:
        EditorWorlds(EngineContext &ctx);

        void initialize();
        void linkViewport(EditorWorld editorWorld, ViewportContext &vctx);
        void update(float deltaTime);
        void render(ViewportContext &vctx);
    };
}}}}