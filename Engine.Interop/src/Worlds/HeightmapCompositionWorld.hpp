#pragma once

#include "../../../Engine/src/World.hpp"
#include "../ViewportContext.hpp"
#include "../EditorState.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class HeightmapCompositionWorld
    {
        EngineContext &ctx;

        struct WorkingWorld
        {
            Engine::World world;
            int cameraEntityId;
            int renderTextureHandle;
            int framebufferHandle;
            int brushQuad_meshRendererInstanceId;
            int quadMaterialHandle;
            bool isFirstRender;

            WorkingWorld(EngineContext &ctx) : world(ctx), isFirstRender(true)
            {
            }
        } working;

        struct StagingWorld
        {
            Engine::World world;
            int cameraEntityId;
            int renderTextureHandle;
            int framebufferHandle;
            int quadMaterialHandle;

            StagingWorld(EngineContext &ctx) : world(ctx)
            {
            }
        } staging;

        int createQuadMaterial(int textureHandle);
        void setupWorkingWorld(int quadMeshHandle);
        void setupStagingWorld(int quadMeshHandle);

    public:
        HeightmapCompositionWorld(EngineContext &ctx);

        void initialize();
        void update(float deltaTime, const EditorState &state, EditorState &newState);
        void compositeHeightmap(const EditorState &state, EditorState &newState);

        int getCompositedTextureHandle() const
        {
            return staging.renderTextureHandle;
        }
    };
}}}}