#pragma once

#include "../ViewportContext.hpp"
#include "ViewportWorld.hpp"
#include "HeightmapPreviewWorld.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class EditorWorlds
    {
    public:
        HeightmapPreviewWorld heightmapPreviewWorld;

        EditorWorlds(EditorMemory *memory);

        void *addView(EditorMemory *memory, ViewportWorld viewportWorld);
        void update(EditorMemory *memory, float deltaTime, EditorInput *input);
        void render(EditorMemory *memory, ViewportContext &vctx);
    };
}}}}