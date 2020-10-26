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