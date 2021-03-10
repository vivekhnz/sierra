#include "EditorWorlds.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    EditorWorlds::EditorWorlds(EditorMemory *memory) : heightmapPreviewWorld(&memory->engine)
    {
    }

    void *EditorWorlds::addView(EditorMemory *memory, ViewportWorld viewportWorld)
    {
        switch (viewportWorld)
        {
        case ViewportWorld::Scene:
            return editorAddSceneView(memory);
        }
        return 0;
    }

    void EditorWorlds::update(EditorMemory *memory, float deltaTime, EditorInput *input)
    {
        if (!memory->isInitialized)
        {
            editorInitialize(memory);

            heightmapPreviewWorld.initialize(
                memory->heightmapCompositionState.working.renderTextureHandle);

            memory->isInitialized = true;
        }
        editorUpdate(memory, deltaTime, input);
    }

    void EditorWorlds::render(EditorMemory *memory, ViewportContext &vctx)
    {
        if (!memory->isInitialized)
            return;

        EditorViewContext view = vctx.getViewContext();
        if (view.width == 0 || view.height == 0)
            return;

        switch (vctx.getWorld())
        {
        case ViewportWorld::Scene:
            editorRenderSceneView(memory, &view);
            break;
        case ViewportWorld::HeightmapPreview:
            heightmapPreviewWorld.render(memory, &view);
            break;
        }
    }
}}}}