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

    void EditorWorlds::linkViewport(ViewportWorld viewportWorld, ViewportContext *vctx)
    {
        uint32 contextId = 0;
        int inputControllerId = vctx->getInputControllerId();

        switch (viewportWorld)
        {
        case ViewportWorld::Scene:
            contextId = sceneWorld.linkViewport(inputControllerId);
            break;
        }

        vctx->setContextId(contextId);
    }

    void EditorWorlds::update(float deltaTime, const EditorState &state, EditorState &newState)
    {
        sceneWorld.update(deltaTime, state, newState);
        heightmapCompositionWorld.update(deltaTime, state, newState);
        heightmapCompositionWorld.compositeHeightmap(state, newState);
    }

    void EditorWorlds::render(EngineMemory *memory, ViewportContext &vctx)
    {
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