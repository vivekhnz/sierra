#include "EditorWorlds.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    EditorWorlds::EditorWorlds(EngineContext &ctx) :
        ctx(&ctx), sceneWorld(ctx), heightmapCompositionWorld(ctx.memory),
        heightmapPreviewWorld(ctx.memory)
    {
    }

    void EditorWorlds::initialize()
    {
        heightmapCompositionWorld.initialize();
        uint32 heightmapTextureHandle = heightmapCompositionWorld.getCompositedTextureHandle();
        uint32 previewTextureHandle = heightmapCompositionWorld.getPreviewTextureHandle();

        sceneWorld.initialize(heightmapTextureHandle, previewTextureHandle);
        heightmapPreviewWorld.initialize(heightmapTextureHandle);
    }

    void EditorWorlds::linkViewport(ViewportWorld viewportWorld, ViewportContext &vctx)
    {
        switch (viewportWorld)
        {
        case ViewportWorld::Scene:
            sceneWorld.linkViewport(vctx);
            break;
        }
    }

    void EditorWorlds::update(float deltaTime, const EditorState &state, EditorState &newState)
    {
        sceneWorld.update(deltaTime, state, newState);
        heightmapCompositionWorld.update(deltaTime, state, newState);
        heightmapCompositionWorld.compositeHeightmap(state, newState);
    }

    void EditorWorlds::render(EngineMemory *memory, ViewportContext &vctx)
    {
        EngineViewContext view = vctx.getViewContext();
        switch (vctx.getWorld())
        {
        case ViewportWorld::Scene:
            sceneWorld.render(
                memory, view.viewportWidth, view.viewportHeight, view.cameraEntityId);
            break;
        case ViewportWorld::HeightmapPreview:
            heightmapPreviewWorld.render(memory, view.viewportWidth, view.viewportHeight);
            break;
        }
    }
}}}}