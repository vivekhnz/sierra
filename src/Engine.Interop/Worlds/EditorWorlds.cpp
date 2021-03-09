#include "EditorWorlds.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    EditorWorlds::EditorWorlds(EngineMemory *memory) :
        sceneWorld(memory), heightmapPreviewWorld(memory)
    {
    }

    void *EditorWorlds::addView(ViewportWorld viewportWorld)
    {
        switch (viewportWorld)
        {
        case ViewportWorld::Scene:
            return sceneWorld.addView();
        }
        return 0;
    }

    void EditorWorlds::update(EditorMemory *editorMemory, float deltaTime, EditorInput *input)
    {
        if (!editorMemory->isInitialized)
        {
            editorInitialize(editorMemory);

            uint32 heightmapTextureHandle =
                editorMemory->heightmapCompositionState.working.renderTextureHandle;
            uint32 previewTextureHandle =
                editorMemory->heightmapCompositionState.preview.renderTextureHandle;

            sceneWorld.initialize(heightmapTextureHandle, previewTextureHandle);
            heightmapPreviewWorld.initialize(heightmapTextureHandle);

            editorMemory->isInitialized = true;
        }

        sceneWorld.update(editorMemory, deltaTime, input);
        editorUpdate(editorMemory, deltaTime, input);
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
            sceneWorld.render(memory, &view);
            break;
        case ViewportWorld::HeightmapPreview:
            heightmapPreviewWorld.render(memory, &view);
            break;
        }
    }
}}}}