#pragma once

#include "../../Engine/World.hpp"
#include "../EditorState.hpp"
#include "../ViewportContext.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class HeightmapPreviewWorld
    {
        EngineContext &ctx;

        int vertexArrayHandle;
        int shaderProgramHandle;
        int heightmapTextureHandle;
        glm::mat4 cameraTransform;

    public:
        HeightmapPreviewWorld(EngineContext &ctx);

        void initialize(int heightmapTextureHandle);
        void render(EngineMemory *memory, int viewportWidth, int viewportHeight);
    };
}}}}