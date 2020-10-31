#pragma once

#include "../../../Engine/src/World.hpp"
#include "../ViewportContext.hpp"
#include "../EditorState.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class HeightmapCompositionWorld
    {
        EngineContext &ctx;
        Engine::World world;

        int cameraEntityId;
        int renderTextureHandle;
        int framebufferHandle;
        int brushQuad_meshRendererInstanceId;

    public:
        HeightmapCompositionWorld(EngineContext &ctx);

        void initialize();
        void update(float deltaTime, const EditorState &state, EditorState &newState);
        void compositeHeightmap();

        int createQuadMaterial();

        int getCompositedTextureHandle() const
        {
            return renderTextureHandle;
        }
    };
}}}}