#ifndef EDITOR_H
#define EDITOR_H

#include <glm/gtc/type_ptr.hpp>

#include "editor_transactions.h"

#define MAX_MATERIAL_COUNT 8
#define MAX_BRUSH_QUADS 2048
#define BRUSH_QUAD_INSTANCE_BUFFER_STRIDE (2 * sizeof(float))
#define BRUSH_QUAD_INSTANCE_BUFFER_SIZE (MAX_BRUSH_QUADS * BRUSH_QUAD_INSTANCE_BUFFER_STRIDE)
#define MAX_OBJECT_INSTANCES 32
#define HEIGHTFIELD_COLUMNS 256
#define HEIGHTFIELD_ROWS 256
#define HEIGHTMAP_WIDTH 2048
#define HEIGHTMAP_HEIGHT 2048

#define PLATFORM_CAPTURE_MOUSE(name) void name()
typedef PLATFORM_CAPTURE_MOUSE(PlatformCaptureMouse);

#define PLATFORM_PUBLISH_TRANSACTION(name)                                                    \
    void name(void *commandBufferBaseAddress, uint64 commandBufferSize)
typedef PLATFORM_PUBLISH_TRANSACTION(PlatformPublishTransaction);

enum TerrainBrushTool
{
    TERRAIN_BRUSH_TOOL_RAISE = 0,
    TERRAIN_BRUSH_TOOL_LOWER = 1,
    TERRAIN_BRUSH_TOOL_FLATTEN = 2,
    TERRAIN_BRUSH_TOOL_SMOOTH = 3
};

struct TerrainMaterialProperties
{
    uint32 albedoTextureAssetId;
    uint32 normalTextureAssetId;
    uint32 displacementTextureAssetId;
    uint32 aoTextureAssetId;
    float textureSizeInWorldUnits;

    float slopeStart;
    float slopeEnd;
    float altitudeStart;
    float altitudeEnd;
};

struct EditorUiState
{
    TerrainBrushTool terrainBrushTool;
    float terrainBrushRadius;
    float terrainBrushFalloff;
    float terrainBrushStrength;

    float sceneLightDirection;
};

struct ObjectTransform
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct TextureAssetBinding
{
    uint32 assetId;
    uint8 version;
};

struct SceneViewState
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

struct SceneState
{
    Heightfield heightfield;
    float heightfieldHeights[HEIGHTFIELD_COLUMNS * HEIGHTFIELD_ROWS] = {0};

    struct TerrainMesh
    {
        uint32 elementCount;
        uint32 vertexBufferHandle;
        uint32 vertexArrayHandle;
    } terrainMesh;

    uint32 tessellationLevelBufferHandle;

    struct RockMesh
    {
        bool isLoaded;
        uint32 elementCount;
        uint32 vertexBufferHandle;
        uint32 vertexArrayHandle;
    } rockMesh;

    uint32 nextObjectId;
    uint32 objectInstanceCount;
    uint32 objectIds[MAX_OBJECT_INSTANCES];
    ObjectTransform objectTransforms[MAX_OBJECT_INSTANCES];
    glm::mat4 objectInstanceBufferData[MAX_OBJECT_INSTANCES];
    uint32 objectInstanceBufferHandle;

    uint32 albedoTextureArrayHandle;
    uint32 normalTextureArrayHandle;
    uint32 displacementTextureArrayHandle;
    uint32 aoTextureArrayHandle;

    TextureAssetBinding albedoTextures[MAX_MATERIAL_COUNT];
    TextureAssetBinding normalTextures[MAX_MATERIAL_COUNT];
    TextureAssetBinding displacementTextures[MAX_MATERIAL_COUNT];
    TextureAssetBinding aoTextures[MAX_MATERIAL_COUNT];

    uint32 materialPropsBufferHandle;

    uint16 *heightmapTextureDataTempBuffer;

    struct WorldState
    {
        glm::vec2 brushPos;
        float brushRadius;
        float brushFalloff;
        SceneViewState *brushCursorVisibleView;

        uint32 materialCount;
        uint32 nextMaterialId;
        uint32 materialIds[MAX_MATERIAL_COUNT];
        GpuMaterialProperties materialProps[MAX_MATERIAL_COUNT];
        uint32 albedoTextureAssetIds[MAX_MATERIAL_COUNT];
        uint32 normalTextureAssetIds[MAX_MATERIAL_COUNT];
        uint32 displacementTextureAssetIds[MAX_MATERIAL_COUNT];
        uint32 aoTextureAssetIds[MAX_MATERIAL_COUNT];
    } worldState;
};

struct HeightmapRenderTexture
{
    uint32 textureHandle;
    uint32 framebufferHandle;
};

struct EditorAssets
{
    uint32 shaderProgramQuad;
    uint32 shaderProgramTerrainCalcTessLevel;
    uint32 shaderProgramTerrainTextured;
    uint32 shaderProgramBrushMask;
    uint32 shaderProgramBrushBlendAddSub;
    uint32 shaderProgramBrushBlendFlatten;
    uint32 shaderProgramBrushBlendSmooth;
    uint32 shaderProgramRock;

    uint32 textureGroundAlbedo;
    uint32 textureGroundNormal;
    uint32 textureGroundDisplacement;
    uint32 textureGroundAo;
    uint32 textureRockAlbedo;
    uint32 textureRockNormal;
    uint32 textureRockDisplacement;
    uint32 textureRockAo;
    uint32 textureSnowAlbedo;
    uint32 textureSnowNormal;
    uint32 textureSnowDisplacement;
    uint32 textureSnowAo;
    uint32 textureVirtualImportedHeightmap;

    uint32 meshRock;
};

struct EditorState
{
    bool isInitialized;

    EditorAssets assets;

    glm::mat4 orthographicCameraTransform;
    uint32 quadVertexArrayHandle;
    uint32 quadFlippedYVertexArrayHandle;

    uint32 importedHeightmapTextureHandle;
    uint8 importedHeightmapTextureVersion;

    HeightmapRenderTexture committedHeightmap;
    HeightmapRenderTexture workingBrushInfluenceMask;
    HeightmapRenderTexture workingHeightmap;
    HeightmapRenderTexture previewBrushInfluenceMask;
    HeightmapRenderTexture previewHeightmap;
    HeightmapRenderTexture temporaryHeightmap;

    bool isEditingHeightmap;
    bool isAdjustingBrushParameters;
    uint32 activeBrushStrokeVertexArrayHandle;
    uint32 activeBrushStrokeInstanceBufferHandle;
    glm::vec2 activeBrushStrokeInstanceBufferData[MAX_BRUSH_QUADS];
    uint32 activeBrushStrokeInstanceCount;
    float activeBrushStrokeInitialHeight;

    EditorUiState uiState;
    SceneState sceneState;

    EditorTransactionQueue transactions;
};

struct EditorMemory
{
    MemoryBlock data;
    uint64 dataStorageUsed;

    PlatformCaptureMouse *platformCaptureMouse;
    PlatformPublishTransaction *platformPublishTransaction;

    EngineApi *engineApi;
    EngineMemory *engineMemory;
};

struct EditorInput
{
    void *activeViewState;

    float scrollOffset;
    glm::vec2 normalizedCursorPos;
    glm::vec2 cursorOffset;
    uint64 pressedButtons;
    uint64 prevPressedButtons;
};

#define BUTTON(name, idx) EDITOR_INPUT_##name## = 1ULL << idx
enum EditorInputButtons : uint64
{
    BUTTON(MOUSE_LEFT, 0),
    BUTTON(MOUSE_MIDDLE, 1),
    BUTTON(MOUSE_RIGHT, 2),
    BUTTON(KEY_SPACE, 3),
    BUTTON(KEY_0, 4),
    BUTTON(KEY_1, 5),
    BUTTON(KEY_2, 6),
    BUTTON(KEY_3, 7),
    BUTTON(KEY_4, 8),
    BUTTON(KEY_5, 9),
    BUTTON(KEY_6, 10),
    BUTTON(KEY_7, 11),
    BUTTON(KEY_8, 12),
    BUTTON(KEY_9, 13),
    BUTTON(KEY_A, 14),
    BUTTON(KEY_B, 15),
    BUTTON(KEY_C, 16),
    BUTTON(KEY_D, 17),
    BUTTON(KEY_E, 18),
    BUTTON(KEY_F, 19),
    BUTTON(KEY_G, 20),
    BUTTON(KEY_H, 21),
    BUTTON(KEY_I, 22),
    BUTTON(KEY_J, 23),
    BUTTON(KEY_K, 24),
    BUTTON(KEY_L, 25),
    BUTTON(KEY_M, 26),
    BUTTON(KEY_N, 27),
    BUTTON(KEY_O, 28),
    BUTTON(KEY_P, 29),
    BUTTON(KEY_Q, 30),
    BUTTON(KEY_R, 31),
    BUTTON(KEY_S, 32),
    BUTTON(KEY_T, 33),
    BUTTON(KEY_U, 34),
    BUTTON(KEY_V, 35),
    BUTTON(KEY_W, 36),
    BUTTON(KEY_X, 37),
    BUTTON(KEY_Y, 38),
    BUTTON(KEY_Z, 39),
    BUTTON(KEY_ESCAPE, 40),
    BUTTON(KEY_ENTER, 41),
    BUTTON(KEY_RIGHT, 42),
    BUTTON(KEY_LEFT, 43),
    BUTTON(KEY_DOWN, 44),
    BUTTON(KEY_UP, 45),
    BUTTON(KEY_F1, 46),
    BUTTON(KEY_F2, 47),
    BUTTON(KEY_F3, 48),
    BUTTON(KEY_F4, 49),
    BUTTON(KEY_F5, 50),
    BUTTON(KEY_F6, 51),
    BUTTON(KEY_F7, 52),
    BUTTON(KEY_F8, 53),
    BUTTON(KEY_F9, 54),
    BUTTON(KEY_F10, 55),
    BUTTON(KEY_F11, 56),
    BUTTON(KEY_F12, 57),
    BUTTON(KEY_LEFT_SHIFT, 58),
    BUTTON(KEY_LEFT_CONTROL, 59),
    BUTTON(KEY_RIGHT_SHIFT, 60),
    BUTTON(KEY_RIGHT_CONTROL, 61),
    BUTTON(KEY_ALT, 62)
};

struct EditorViewContext
{
    void *viewState;
    uint32 x;
    uint32 y;
    uint32 width;
    uint32 height;
};

#define EDITOR_UPDATE(name)                                                                   \
    void name(EditorMemory *memory, float deltaTime, EditorInput *input)
typedef EDITOR_UPDATE(EditorUpdate);

#define EDITOR_RENDER_SCENE_VIEW(name) void name(EditorMemory *memory, EditorViewContext *view)
typedef EDITOR_RENDER_SCENE_VIEW(EditorRenderSceneView);

#define EDITOR_RENDER_HEIGHTMAP_PREVIEW(name)                                                 \
    void name(EditorMemory *memory, EditorViewContext *view)
typedef EDITOR_RENDER_HEIGHTMAP_PREVIEW(EditorRenderHeightmapPreview);

#define EDITOR_GET_IMPORTED_HEIGHTMAP_ASSET_ID(name) uint32 name(EditorMemory *memory)
typedef EDITOR_GET_IMPORTED_HEIGHTMAP_ASSET_ID(EditorGetImportedHeightmapAssetId);

#define EDITOR_GET_UI_STATE(name) EditorUiState *name(EditorMemory *memory)
typedef EDITOR_GET_UI_STATE(EditorGetUiState);

#define EDITOR_ADD_MATERIAL(name)                                                             \
    void name(EditorMemory *memory, TerrainMaterialProperties props)
typedef EDITOR_ADD_MATERIAL(EditorAddMaterial);

#define EDITOR_DELETE_MATERIAL(name) void name(EditorMemory *memory, uint32 index)
typedef EDITOR_DELETE_MATERIAL(EditorDeleteMaterial);

#define EDITOR_SWAP_MATERIAL(name)                                                            \
    void name(EditorMemory *memory, uint32 indexA, uint32 indexB)
typedef EDITOR_SWAP_MATERIAL(EditorSwapMaterial);

#define EDITOR_SET_MATERIAL_TEXTURE(name)                                                     \
    void name(EditorMemory *memory, uint32 materialId,                                        \
        TerrainMaterialTextureType textureType, uint32 assetId)
typedef EDITOR_SET_MATERIAL_TEXTURE(EditorSetMaterialTexture);

#define EDITOR_SET_MATERIAL_PROPERTIES(name)                                                  \
    void name(EditorMemory *memory, uint32 materialId, float textureSize, float slopeStart,   \
        float slopeEnd, float altitudeStart, float altitudeEnd)
typedef EDITOR_SET_MATERIAL_PROPERTIES(EditorSetMaterialProperties);

#define EDITOR_ADD_OBJECT(name) void name(EditorMemory *memory)
typedef EDITOR_ADD_OBJECT(EditorAddObject);

#define EDITOR_SET_OBJECT_TRANSFORM(name)                                                     \
    void name(EditorMemory *memory, uint32 objectId, float positionX, float positionY,        \
        float positionZ, float rotationX, float rotationY, float rotationZ, float scaleX,     \
        float scaleY, float scaleZ)
typedef EDITOR_SET_OBJECT_TRANSFORM(EditorSetObjectTransform);

#endif