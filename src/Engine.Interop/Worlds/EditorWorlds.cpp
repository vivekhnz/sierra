#include "EditorWorlds.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    EditorWorlds::EditorWorlds(EngineContext &ctx) :
        sceneWorld(ctx), heightmapCompositionWorld(ctx), heightmapPreviewWorld(ctx)
    {
    }

    void EditorWorlds::initialize()
    {
        heightmapCompositionWorld.initialize();
        int heightmapTextureHandle = heightmapCompositionWorld.getCompositedTextureHandle();

        sceneWorld.initialize(heightmapTextureHandle);
        heightmapPreviewWorld.initialize(heightmapTextureHandle);
    }

    void EditorWorlds::linkViewport(ViewportWorld viewportWorld, ViewportContext &vctx)
    {
        switch (viewportWorld)
        {
        case ViewportWorld::Scene:
            sceneWorld.linkViewport(vctx);
            break;
        case ViewportWorld::HeightmapPreview:
            heightmapPreviewWorld.linkViewport(vctx);
            break;
        }
    }

    void EditorWorlds::update(float deltaTime, const EditorState &state, EditorState &newState)
    {
        sceneWorld.update(deltaTime, state, newState);
        heightmapCompositionWorld.update(deltaTime, state, newState);
        heightmapPreviewWorld.update(deltaTime, state, newState);
        heightmapCompositionWorld.compositeHeightmap(state, newState);
    }

    void EditorWorlds::render(ViewportContext &vctx)
    {
        EngineViewContext view = vctx.getViewContext();
        switch (vctx.getWorld())
        {
        case ViewportWorld::Scene:
            sceneWorld.render(view);
            break;
        case ViewportWorld::HeightmapPreview:
            heightmapPreviewWorld.render(view);
            break;
        }
    }
}}}}