#pragma once

#include "../EditorState.hpp"
#include "../../Engine/EngineViewContext.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class HeightmapPreviewWorld
    {
        EngineMemory *memory;

        uint32 vertexArrayHandle;
        uint32 heightmapTextureHandle;
        glm::mat4 cameraTransform;

    public:
        HeightmapPreviewWorld(EngineMemory *memory);

        void initialize(uint32 heightmapTextureHandle);
        void render(EngineMemory *memory, EngineViewContext *vctx);
    };
}}}}