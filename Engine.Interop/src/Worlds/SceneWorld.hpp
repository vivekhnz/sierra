#pragma once

#include "../../../Engine/src/World.hpp"
#include "../EditorState.hpp"
#include "../ViewportContext.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class SceneWorld
    {
        struct OperationState
        {
            EditorTool selectedTool;
            bool isBrushActive;
            bool isDiscardingStroke;
            glm::vec2 brushPosition;
        };

        EngineContext &ctx;
        Engine::World world;

        int heightmapTextureHandle;
        int terrainMeshRendererInstanceId;
        int terrainColliderInstanceId;
        std::vector<int> orbitCameraIds;

        void *heightmapTextureDataTempBuffer;

        OperationState getCurrentOperation(EditStatus currentEditStatus);
        EditStatus getNextEditStatus(
            EditStatus currentEditStatus, bool isBrushActive, bool isDiscardingStroke);

    public:
        SceneWorld(EngineContext &ctx);
        ~SceneWorld();

        void initialize(int heightmapTextureHandle);
        void linkViewport(ViewportContext &vctx);
        void update(float deltaTime, const EditorState &state, EditorState &newState);
        void render(EngineViewContext &vctx);
    };
}}}}