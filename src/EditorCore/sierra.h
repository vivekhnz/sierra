#ifndef SIERRA_H
#define SIERRA_H

#include "sierra_platform.h"
#include "sierra_math.h"
#include "sierra_renderer_common.h"
#include "sierra_renderer.h"
#include "sierra_assets.h"
#include "sierra_transactions.h"
#include "sierra_heightmap.h"

// feature flags
#define FEATURE_TERRAIN_MATERIALS 0
#define FEATURE_OBJECTS 0

#define MAX_MATERIAL_COUNT 8
#define BRUSH_QUAD_INSTANCE_BUFFER_STRIDE (2 * sizeof(float))
#define BRUSH_QUAD_INSTANCE_BUFFER_SIZE (MAX_BRUSH_QUADS * BRUSH_QUAD_INSTANCE_BUFFER_STRIDE)
#define MAX_OBJECT_INSTANCES 32

#define HEIGHTFIELD_SAMPLES_PER_EDGE 32
#define HEIGHTMAP_DIM 1024
#define HEIGHTMAP_OVERLAP_IN_TEXELS 42.0f
#define TERRAIN_TILE_LENGTH_IN_WORLD_UNITS 64.0f

enum EditorContext
{
    EDITOR_CTX_TERRAIN = 0,
    EDITOR_CTX_OBJECTS = 1,
    EDITOR_CTX_SCENE = 2
};

enum TerrainBrushTool
{
    TERRAIN_BRUSH_TOOL_RAISE = 0,
    TERRAIN_BRUSH_TOOL_LOWER = 1,
    TERRAIN_BRUSH_TOOL_FLATTEN = 2,
    TERRAIN_BRUSH_TOOL_SMOOTH = 3
};

struct TerrainMaterialProperties
{
    AssetHandle albedoTextureAssetHandle;
    AssetHandle normalTextureAssetHandle;
    AssetHandle displacementTextureAssetHandle;
    AssetHandle aoTextureAssetHandle;
    float textureSizeInWorldUnits;

    float slopeStart;
    float slopeEnd;
    float altitudeStart;
    float altitudeEnd;
};

struct EditorDebugState
{
    bool showTerrainRaycastVis;
    bool showTerrainTileBounds;
    bool showTerrainTileHeightmap;
};

struct EditorUiState
{
    EditorContext currentContext;

    uint32 *selectedObjectIds;
    uint32 selectedObjectCount;

    TerrainBrushTool terrainBrushTool;
    float terrainBrushRadius;
    float terrainBrushFalloff;
    float terrainBrushStrength;

    float sceneLightDirection;

    EditorDebugState debugState;
};

struct ObjectTransform
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

enum InteractionTargetType
{
    INTERACTION_TARGET_NONE,

    INTERACTION_TARGET_CAMERA,
    INTERACTION_TARGET_TERRAIN,
    INTERACTION_TARGET_OBJECT,
    INTERACTION_TARGET_MANIPULATOR
};
struct InteractionTarget
{
    InteractionTargetType type;
    void *id;
};
struct Interaction
{
    InteractionTarget target;
    void *state;
};
struct InteractionState
{
    Interaction hot;
    Interaction nextHot;
    Interaction active;
    MemoryArena activeArena;
};

struct SceneViewState
{
    float orbitCameraDistance;
    float orbitCameraYaw;
    float orbitCameraPitch;
    glm::vec3 cameraPos;
    glm::vec3 cameraLookAt;

    glm::mat4 cameraTransform;
    glm::mat4 invCameraTransform;

    RenderTarget *sceneRenderTarget;
    RenderTarget *selectionRenderTarget;
    RenderTarget *pickingRenderTarget;

    InteractionState interactionState;
};

struct TerrainTile
{
    glm::vec2 center;
    float maxHeight;
    float *heights;

    RenderTarget *committedHeightmap;
    RenderTarget *workingBrushInfluenceMask;
    RenderTarget *workingPrevBrushInfluenceMask;
    RenderTarget *workingHeightmap;
    RenderTarget *previewBrushInfluenceMask;
    RenderTarget *previewHeightmap;

    TerrainTile *tileToLeft;
    TerrainTile *tileToRight;
    TerrainTile *tileBelow;
};

struct SceneState
{
    TerrainTile *terrainTiles;
    uint32 terrainTileCount;

    uint32 nextObjectId;
    RenderMeshInstance objectInstanceData[MAX_OBJECT_INSTANCES];
    uint32 objectInstanceCount;

    uint32 nextMaterialId;
};

struct EditorAssets
{
    AssetHandle quadShaderBrushMask;
    AssetHandle quadShaderBrushBlendAddSub;
    AssetHandle quadShaderBrushBlendFlatten;
    AssetHandle quadShaderBrushBlendSmooth;
    AssetHandle quadShaderOutline;
    AssetHandle quadShaderIdVisualiser;
    AssetHandle quadShaderId;
    AssetHandle quadShaderTextureMultiplied;
    AssetHandle meshShaderId;
    AssetHandle meshShaderRock;
    AssetHandle terrainShaderTextured;

    AssetHandle textureGroundAlbedo;
    AssetHandle textureGroundNormal;
    AssetHandle textureGroundDisplacement;
    AssetHandle textureGroundAo;
    AssetHandle textureRockAlbedo;
    AssetHandle textureRockNormal;
    AssetHandle textureRockDisplacement;
    AssetHandle textureRockAo;
    AssetHandle textureSnowAlbedo;
    AssetHandle textureSnowNormal;
    AssetHandle textureSnowDisplacement;
    AssetHandle textureSnowAo;
    AssetHandle textureVirtualImportedHeightmap;

    AssetHandle meshRock;
};

struct EditorDocumentState
{
    uint32 materialCount;
    uint32 materialIds[MAX_MATERIAL_COUNT];
    RenderTerrainMaterial materials[MAX_MATERIAL_COUNT];

    uint32 objectInstanceCount;
    uint32 objectIds[MAX_OBJECT_INSTANCES];
    ObjectTransform objectTransforms[MAX_OBJECT_INSTANCES];
};

struct EditorState
{
    bool isInitialized;

    MemoryArena rendererArena;
    MemoryArena assetsArena;

    EditorAssets editorAssets;
    RenderContext *renderCtx;
    Assets *assetCtx;

    TextureHandle importedHeightmapTexture;
    uint8 importedHeightmapTextureVersion;

    RenderTarget *temporaryHeightmap;

    // state related to the user interface e.g. current brush tool, brush radius etc.
    // can be directly read from and written to by the editor UI
    EditorUiState uiState;

    // state related to the document currently being edited e.g. object transforms
    // all changes are made via transactions to support undo/redo
    EditorDocumentState docState;
    TransactionState transactions;

    // committed document state + any active transactions
    EditorDocumentState previewDocState;

    SceneState sceneState;
};

struct EditorMemory
{
    MemoryArena arena;
    EditorPlatformApi platformApi;
};

struct EditorInput
{
    bool isActive;
    float scrollOffset;
    glm::vec2 cursorPos;
    uint64 pressedButtons;
    uint64 prevPressedButtons;
    glm::vec2 capturedCursorDelta;

    bool isMouseCaptured;
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
    BUTTON(KEY_ALT, 60),
    BUTTON(KEY_DELETE, 61)
};

struct EditorViewContext
{
    void *viewState;
    uint32 width;
    uint32 height;
};

#define EDITOR_UPDATE(name) void name(EditorMemory *memory, float deltaTime)
typedef EDITOR_UPDATE(EditorUpdate);

#define EDITOR_RENDER_SCENE_VIEW(name)                                                                            \
    void name(EditorMemory *memory, EditorViewContext *view, float deltaTime, EditorInput *input)
typedef EDITOR_RENDER_SCENE_VIEW(EditorRenderSceneView);

#define EDITOR_RENDER_HEIGHTMAP_PREVIEW(name)                                                                     \
    void name(EditorMemory *memory, EditorViewContext *view, float deltaTime, EditorInput *input)
typedef EDITOR_RENDER_HEIGHTMAP_PREVIEW(EditorRenderHeightmapPreview);

#define EDITOR_GET_IMPORTED_HEIGHTMAP_ASSET_HANDLE(name) AssetHandle name(EditorMemory *memory)
typedef EDITOR_GET_IMPORTED_HEIGHTMAP_ASSET_HANDLE(EditorGetImportedHeightmapAssetHandle);

#define EDITOR_SAVE_HEIGHTMAP(name) void name(EditorMemory *memory, char *filePath)
typedef EDITOR_SAVE_HEIGHTMAP(EditorSaveHeightmap);

#define EDITOR_GET_UI_STATE(name) EditorUiState *name(EditorMemory *memory)
typedef EDITOR_GET_UI_STATE(EditorGetUiState);

#define EDITOR_ADD_MATERIAL(name) void name(EditorMemory *memory, TerrainMaterialProperties props)
typedef EDITOR_ADD_MATERIAL(EditorAddMaterial);

#define EDITOR_DELETE_MATERIAL(name) void name(EditorMemory *memory, uint32 index)
typedef EDITOR_DELETE_MATERIAL(EditorDeleteMaterial);

#define EDITOR_SWAP_MATERIAL(name) void name(EditorMemory *memory, uint32 indexA, uint32 indexB)
typedef EDITOR_SWAP_MATERIAL(EditorSwapMaterial);

#define EDITOR_SET_MATERIAL_TEXTURE(name)                                                                         \
    void name(                                                                                                    \
        EditorMemory *memory, uint32 materialId, TerrainMaterialTextureType textureType, AssetHandle assetHandle)
typedef EDITOR_SET_MATERIAL_TEXTURE(EditorSetMaterialTexture);

#define EDITOR_SET_MATERIAL_PROPERTIES(name)                                                                      \
    void name(EditorMemory *memory, uint32 materialId, float textureSize, float slopeStart, float slopeEnd,       \
        float altitudeStart, float altitudeEnd)
typedef EDITOR_SET_MATERIAL_PROPERTIES(EditorSetMaterialProperties);

#define EDITOR_GET_OBJECT_PROPERTY(name) float name(EditorMemory *memory, uint32 objectId, ObjectProperty property)
typedef EDITOR_GET_OBJECT_PROPERTY(EditorGetObjectProperty);

#define EDITOR_BEGIN_TRANSACTION(name) Transaction *name(EditorMemory *memory)
typedef EDITOR_BEGIN_TRANSACTION(EditorBeginTransaction);

#define EDITOR_CLEAR_TRANSACTION(name) void name(Transaction *tx)
typedef EDITOR_CLEAR_TRANSACTION(EditorClearTransaction);

#define EDITOR_COMMIT_TRANSACTION(name) void name(Transaction *tx)
typedef EDITOR_COMMIT_TRANSACTION(EditorCommitTransaction);

#define EDITOR_DISCARD_TRANSACTION(name) void name(Transaction *tx)
typedef EDITOR_DISCARD_TRANSACTION(EditorDiscardTransaction);

#define EDITOR_ADD_OBJECT(name) void name(EditorMemory *memory, Transaction *tx)
typedef EDITOR_ADD_OBJECT(EditorAddObject);

#define EDITOR_DELETE_OBJECT(name) void name(Transaction *tx, uint32 objectId)
typedef EDITOR_DELETE_OBJECT(EditorDeleteObject);

#define EDITOR_SET_OBJECT_PROPERTY(name)                                                                          \
    void name(Transaction *tx, uint32 objectId, ObjectProperty property, float value)
typedef EDITOR_SET_OBJECT_PROPERTY(EditorSetObjectProperty);

#endif