#pragma once

#include "../../Engine/World.hpp"
#include "../ViewportContext.hpp"
#include "../EditorState.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class HeightmapCompositionWorld
    {
        EngineContext &ctx;

        glm::mat4 cameraTransform;
        uint32 quadVertexArrayHandle;

        struct WorkingWorld
        {
            static const int MAX_BRUSH_QUADS = 2048;
            static const int BRUSH_QUAD_INSTANCE_BUFFER_STRIDE = 2 * sizeof(float);
            static const int BRUSH_QUAD_INSTANCE_BUFFER_SIZE =
                MAX_BRUSH_QUADS * BRUSH_QUAD_INSTANCE_BUFFER_STRIDE;

            uint32 renderTextureHandle;
            int framebufferHandle;
            uint32 importedHeightmapTextureHandle;
            uint32 baseHeightmapTextureHandle;
            uint32 brushQuadVertexArrayHandle;

            int brushQuadInstanceBufferHandle;
            float brushQuadInstanceBufferData[MAX_BRUSH_QUADS * 2];
            int brushInstanceCount;
        } working;

        struct StagingWorld
        {
            uint32 renderTextureHandle;
            int framebufferHandle;
        } staging;

        void setupWorkingWorld();
        void setupStagingWorld();
        void addBrushInstance(glm::vec2 pos);

    public:
        HeightmapCompositionWorld(EngineContext &ctx);

        void initialize();
        void update(float deltaTime, const EditorState &state, EditorState &newState);
        void compositeHeightmap(const EditorState &state, EditorState &newState);
        void updateImportedHeightmapTexture(TextureAsset *asset);

        uint32 getCompositedTextureHandle() const
        {
            return working.renderTextureHandle;
        }
    };
}}}}