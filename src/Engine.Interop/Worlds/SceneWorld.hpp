#pragma once

#include "../../Engine/terrain_renderer.h"
#include "../../Engine/World.hpp"
#include "../EditorState.hpp"
#include "../ViewportContext.hpp"

#define MAX_SCENE_VIEWS 8

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class SceneWorld
    {
        struct OperationState
        {
            InteractionMode mode;
            EditorTool tool;
            bool isBrushActive;
            bool isDiscardingStroke;
            glm::vec2 brushPosition;
            float brushRadiusIncrease;
            float brushFalloffIncrease;
        };

        struct ViewState
        {
            int inputControllerId;
            float orbitCameraDistance;
            float orbitCameraYaw;
            float orbitCameraPitch;
            glm::vec3 cameraPos;
            glm::vec3 cameraLookAt;
            glm::mat4 cameraTransform;
        };

        EngineContext &ctx;
        Engine::World world;

        int heightmapTextureHandle;
        int terrainMeshRendererInstanceId;
        int terrainColliderInstanceId;

        void *heightmapTextureDataTempBuffer;

        ViewState viewStates[MAX_SCENE_VIEWS];
        int viewStateCount = 0;

        bool updateViewState(ViewState *viewState, float deltaTime);
        OperationState getCurrentOperation(const EditorState &prevState);
        HeightmapStatus getNextHeightmapStatus(HeightmapStatus currentHeightmapStatus,
            bool isBrushActive,
            bool isDiscardingStroke);

    public:
        SceneWorld(EngineContext &ctx);
        ~SceneWorld();

        void initialize(int heightmapTextureHandle);
        void linkViewport(ViewportContext &vctx);
        void update(float deltaTime, const EditorState &state, EditorState &newState);
        void render(
            EngineMemory *memory, uint32 viewportWidth, uint32 viewportHeight, int32 viewId);
    };
}}}}