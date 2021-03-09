#pragma once

#include "../ViewportContext.hpp"
#include "ViewportWorld.hpp"
#include "SceneWorld.hpp"
#include "HeightmapPreviewWorld.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class EditorWorlds
    {
    public:
        SceneWorld sceneWorld;
        HeightmapPreviewWorld heightmapPreviewWorld;

        EditorWorlds(EngineMemory *memory);

        void *addView(ViewportWorld viewportWorld);
        void update(EditorMemory *editorMemory, float deltaTime, EditorInput *input);
        void render(EditorMemory *memory, ViewportContext &vctx);
    };
}}}}