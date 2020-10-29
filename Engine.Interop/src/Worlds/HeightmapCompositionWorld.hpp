#pragma once

#include "../../../Engine/src/World.hpp"
#include "../ViewportContext.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class HeightmapCompositionWorld
    {
        EngineContext &ctx;
        Engine::World world;

        int cameraEntityId;
        int renderTextureHandle;
        int framebufferHandle;
        int brushQuad_meshRendererInstanceId;

        float brushQuad_x;
        float brushQuad_y;

    public:
        HeightmapCompositionWorld(EngineContext &ctx);

        void initialize();
        void update(float deltaTime);
        void compositeHeightmap();

        int createQuadMaterial();

        int getCompositedTextureHandle() const
        {
            return renderTextureHandle;
        }
    };
}}}}