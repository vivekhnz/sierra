#ifndef EDITOR_H
#define EDITOR_H

#include <glm/gtc/type_ptr.hpp>
#include "../Engine/terrain_platform.h"
#include "../Engine/terrain_assets.h"
#include "../Engine/terrain_heightfield.h"

#define MAX_MATERIAL_COUNT 8
#define MAX_BRUSH_QUADS 2048
#define BRUSH_QUAD_INSTANCE_BUFFER_STRIDE (2 * sizeof(float))
#define BRUSH_QUAD_INSTANCE_BUFFER_SIZE (MAX_BRUSH_QUADS * BRUSH_QUAD_INSTANCE_BUFFER_STRIDE)
#define MAX_SCENE_VIEWS 8
#define HEIGHTFIELD_COLUMNS 256
#define HEIGHTFIELD_ROWS 256
#define HEIGHTMAP_WIDTH 2048
#define HEIGHTMAP_HEIGHT 2048

#define PLATFORM_CAPTURE_MOUSE(name) void name()
typedef PLATFORM_CAPTURE_MOUSE(PlatformCaptureMouse);

enum HeightmapStatus
{
    HEIGHTMAP_STATUS_INITIALIZING = 0,
    HEIGHTMAP_STATUS_IDLE = 1,
    HEIGHTMAP_STATUS_EDITING = 2,
    HEIGHTMAP_STATUS_COMMITTING = 3,
    HEIGHTMAP_STATUS_DISCARDING = 4
};

enum InteractionMode
{
    INTERACTION_MODE_MOVE_CAMERA = 0,
    INTERACTION_MODE_MODIFY_BRUSH_RADIUS = 1,
    INTERACTION_MODE_MODIFY_BRUSH_FALLOFF = 2,
    INTERACTION_MODE_MODIFY_BRUSH_STRENGTH = 3,
    INTERACTION_MODE_PAINT_BRUSH_STROKE = 4
};

enum EditorTool
{
    EDITOR_TOOL_RAISE_TERRAIN = 0,
    EDITOR_TOOL_LOWER_TERRAIN = 1
};

struct MaterialProperties
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

struct EditorState
{
    HeightmapStatus heightmapStatus;
    InteractionMode mode;
    EditorTool tool;
    glm::vec2 currentBrushPos;
    float brushRadius;
    float brushFalloff;
    float brushStrength;
    float lightDirection;
    uint32 materialCount;
    MaterialProperties materialProps[MAX_MATERIAL_COUNT];
};

struct HeightmapCompositionState
{
    glm::mat4 cameraTransform;
    uint32 quadVertexArrayHandle;

    struct WorkingWorld
    {
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

    struct PreviewWorld
    {
        uint32 renderTextureHandle;
        int framebufferHandle;
    } preview;
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

    SceneViewState viewStates[MAX_SCENE_VIEWS];
    int viewStateCount = 0;

    struct WorldState
    {
        glm::vec2 brushPos;
        float brushRadius;
        float brushFalloff;
        SceneViewState *brushCursorVisibleView;
        bool isPreviewingChanges;

        uint32 materialCount;
        GpuMaterialProperties materialProps[MAX_MATERIAL_COUNT];
        uint32 albedoTextureAssetIds[MAX_MATERIAL_COUNT];
        uint32 normalTextureAssetIds[MAX_MATERIAL_COUNT];
        uint32 displacementTextureAssetIds[MAX_MATERIAL_COUNT];
        uint32 aoTextureAssetIds[MAX_MATERIAL_COUNT];
    } worldState;
};

struct HeightmapPreviewState
{
    uint32 vertexArrayHandle;
    glm::mat4 cameraTransform;
};

struct EditorMemory
{
    bool isInitialized;

    PlatformCaptureMouse *platformCaptureMouse;

    EditorState currentState;
    EditorState newState;
    HeightmapCompositionState heightmapCompositionState;
    SceneState sceneState;
    HeightmapPreviewState heightmapPreviewState;

    MemoryBlock data;
    uint64 dataStorageUsed;

    EngineMemory engine;
};

struct EditorInput
{
    void *activeViewState;

    float scrollOffset;
    glm::vec2 normalizedCursorPos;
    glm::vec2 cursorOffset;
    uint8 pressedMouseButtons;
    uint8 prevPressedMouseButtons;

    uint64 pressedKeys;
    uint64 prevPressedKeys;
};

struct EditorViewContext
{
    void *viewState;
    uint32 width;
    uint32 height;
};

void editorUpdate(EditorMemory *memory, float deltaTime, EditorInput *input);
void editorShutdown(EditorMemory *memory);

void *editorAddSceneView(EditorMemory *memory);
void editorRenderSceneView(EditorMemory *memory, EditorViewContext *view);

void editorUpdateImportedHeightmapTexture(EditorMemory *memory, TextureAsset *asset);

void editorRenderHeightmapPreview(EditorMemory *memory, EditorViewContext *view);

#endif