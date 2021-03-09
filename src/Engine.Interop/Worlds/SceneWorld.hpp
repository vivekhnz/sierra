#pragma once

#include "../../Engine/terrain_renderer.h"
#include "../../Engine/terrain_heightfield.h"
#include "../EditorViewContext.hpp"
#include "../EditorState.hpp"
#include "../editor.h"

#define MAX_SCENE_VIEWS 8
#define HEIGHTFIELD_COLUMNS 256
#define HEIGHTFIELD_ROWS 256

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
            float brushStrengthIncrease;
        };

        struct ViewState
        {
            float orbitCameraDistance;
            float orbitCameraYaw;
            float orbitCameraPitch;
            glm::vec3 cameraPos;
            glm::vec3 cameraLookAt;
            glm::mat4 cameraTransform;
        };

        struct GpuMaterialProperties
        {
            glm::vec2 textureSizeInWorldUnits;
            glm::vec2 _padding;
            glm::vec4 rampParams;
        };

        struct WorldState
        {
            glm::vec2 brushPos;
            float brushRadius;
            float brushFalloff;
            bool isBrushHighlightVisible;
            bool isPreviewingChanges;

            uint32 materialCount;
            GpuMaterialProperties materialProps[MAX_MATERIAL_COUNT];
            uint32 albedoTextureAssetIds[MAX_MATERIAL_COUNT];
            uint32 normalTextureAssetIds[MAX_MATERIAL_COUNT];
            uint32 displacementTextureAssetIds[MAX_MATERIAL_COUNT];
            uint32 aoTextureAssetIds[MAX_MATERIAL_COUNT];
        };

        struct TerrainMesh
        {
            uint32 elementCount;
            uint32 vertexBufferHandle;
            uint32 vertexArrayHandle;
        };

        struct TextureAssetBinding
        {
            uint32 assetId;
            uint8 version;
        };

        EngineMemory *memory;

        Heightfield heightfield;
        float heightfieldHeights[HEIGHTFIELD_COLUMNS * HEIGHTFIELD_ROWS] = {0};

        uint32 heightmapTextureHandle;
        uint32 previewTextureHandle;

        TerrainMesh terrainMesh;

        uint32 tessellationLevelBufferHandle;

        uint32 albedoTextureArrayHandle;
        uint32 normalTextureArrayHandle;
        uint32 displacementTextureArrayHandle;
        uint32 aoTextureArrayHandle;

        TextureAssetBinding albedoTextures[MAX_MATERIAL_COUNT];
        TextureAssetBinding normalTextures[MAX_MATERIAL_COUNT];
        TextureAssetBinding displacementTextures[MAX_MATERIAL_COUNT];
        TextureAssetBinding aoTextures[MAX_MATERIAL_COUNT];

        uint32 materialPropsBufferHandle;

        void *heightmapTextureDataTempBuffer;

        ViewState viewStates[MAX_SCENE_VIEWS];
        int viewStateCount = 0;
        WorldState worldState;

        bool updateViewState(ViewState *viewState, float deltaTime, EditorInput *input);
        OperationState getCurrentOperation(const EditorState &prevState, EditorInput *input);
        HeightmapStatus getNextHeightmapStatus(HeightmapStatus currentHeightmapStatus,
            bool isBrushActive,
            bool isDiscardingStroke);

    public:
        SceneWorld(EngineMemory *memory);
        ~SceneWorld();

        void initialize(uint32 heightmapTextureHandle, uint32 previewTextureHandle);
        void *addView();
        void update(EditorMemory *editorMemory,
            float deltaTime,
            const EditorState &state,
            EditorState &newState,
            EditorInput *input);
        void render(EngineMemory *memory, EditorViewContext *vctx);
    };
}}}}