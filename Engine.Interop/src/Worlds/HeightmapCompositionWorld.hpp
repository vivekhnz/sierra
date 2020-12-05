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
            static const int MAX_BRUSH_QUADS = 1024;
            static const int BRUSH_QUAD_INSTANCE_BUFFER_STRIDE = 2 * sizeof(float);
            static const int BRUSH_QUAD_INSTANCE_BUFFER_SIZE =
                MAX_BRUSH_QUADS * BRUSH_QUAD_INSTANCE_BUFFER_STRIDE;

            Engine::World world;
            int cameraEntityId;
            int renderTextureHandle;
            int framebufferHandle;
            int brushQuad_meshRendererInstanceId;
            int quadMaterialHandle;

            int brushQuad_instanceBufferHandle;
            float brushQuad_instanceBufferData[MAX_BRUSH_QUADS * 2];
            int brushInstanceCount;

            WorkingWorld(EngineContext &ctx) : world(ctx)
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