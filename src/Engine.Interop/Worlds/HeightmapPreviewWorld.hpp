#pragma once

#include "../../Engine/World.hpp"
#include "../EditorState.hpp"
#include "../ViewportContext.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class HeightmapPreviewWorld
    {
        EngineContext &ctx;

        uint32 vertexArrayHandle;
        uint32 shaderProgramHandle;
        uint32 heightmapTextureHandle;
        glm::mat4 cameraTransform;

    public:
        HeightmapPreviewWorld(EngineContext &ctx);

        void initialize(uint32 heightmapTextureHandle);
        void render(MemoryBlock *memory, uint32 viewportWidth, uint32 viewportHeight);
    };
}}}}