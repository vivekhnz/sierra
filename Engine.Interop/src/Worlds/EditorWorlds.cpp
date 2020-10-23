#include "EditorWorlds.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    EditorWorlds::EditorWorlds(EngineContext &ctx) : sceneWorld(ctx), heightmapWorld(ctx)
    {
    }

    void EditorWorlds::initialize()
    {
        sceneWorld.initialize();
        heightmapWorld.initialize();
    }

    void EditorWorlds::linkViewport(EditorWorld editorWorld, ViewportContext &vctx)
    {
        switch (editorWorld)
        {
        case EditorWorld::Scene:
            sceneWorld.linkViewport(vctx);
            break;
        case EditorWorld::Heightmap:
            heightmapWorld.linkViewport(vctx);
            break;
        }
    }

    void EditorWorlds::update(float deltaTime)
    {
        sceneWorld.update(deltaTime);
        heightmapWorld.update(deltaTime);
    }

    void EditorWorlds::render(ViewportContext &vctx)
    {
        EngineViewContext view = vctx.getViewContext();
        switch (vctx.getWorld())
        {
        case EditorWorld::Scene:
            sceneWorld.render(view);
            break;
        case EditorWorld::Heightmap:
            heightmapWorld.render(view);
            break;
        }
    }
}}}}