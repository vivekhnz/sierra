#include "EditorWorlds.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    EditorWorlds::EditorWorlds(EngineContext &ctx) :
        sceneWorld(ctx), heightmapCompositionWorld(ctx), heightmapPreviewWorld(ctx)
    {
    }

    void EditorWorlds::initialize()
    {
        sceneWorld.initialize();
        heightmapCompositionWorld.initialize();
        heightmapPreviewWorld.initialize(
            heightmapCompositionWorld.getCompositedTextureHandle());
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

    void EditorWorlds::update(float deltaTime)
    {
        sceneWorld.update(deltaTime);
        heightmapCompositionWorld.update(deltaTime);
        heightmapPreviewWorld.update(deltaTime);

        heightmapCompositionWorld.compositeHeightmap();
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