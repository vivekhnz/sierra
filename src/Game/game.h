#ifndef GAME_H
#define GAME_H

#include <glm/gtc/type_ptr.hpp>
#include "../Engine/engine.h"

#define HEIGHTFIELD_SAMPLES_PER_EDGE 256
#define MATERIAL_COUNT 3

struct GameAssets
{
    AssetHandle terrainShaderWireframe;
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
    AssetHandle textureVirtualHeightmap;
};

struct GameState
{
    MemoryArena assetsArena;

    RenderContext *renderCtx;
    Assets *engineAssets;

    GameAssets gameAssets;

    bool isOrbitCameraMode;
    bool isWireframeMode;
    bool isLightingEnabled;
    bool isAlbedoEnabled;
    bool isNormalMapEnabled;
    bool isAOMapEnabled;
    bool isDisplacementMapEnabled;

    float orbitCameraDistance;
    glm::vec3 orbitCameraPos;
    glm::vec3 orbitCameraLookAt;
    float orbitCameraPitch;
    float orbitCameraYaw;

    float firstPersonCameraYaw;
    float firstPersonCameraPitch;
    glm::vec3 firstPersonCameraPos;
    glm::vec3 firstPersonCameraLookAt;

    Heightfield heightfield;
    float heightfieldHeights[HEIGHTFIELD_SAMPLES_PER_EDGE * HEIGHTFIELD_SAMPLES_PER_EDGE];

    TextureHandle heightmapTexture;
    uint8 heightmapTextureVersion;

    RenderTerrainMaterial materials[MATERIAL_COUNT];
};

struct PlatformReadFileResult
{
    uint64 size;
    void *data;
};

#define PLATFORM_GET_ASSET_ABSOLUTE_PATH(name) void name(const char *relativePath, char *absolutePath)
typedef PLATFORM_GET_ASSET_ABSOLUTE_PATH(PlatformGetAssetAbsolutePath);

#define PLATFORM_READ_FILE(name) PlatformReadFileResult name(const char *path)
typedef PLATFORM_READ_FILE(PlatformReadFile);

#define PLATFORM_FREE_MEMORY(name) void name(void *data)
typedef PLATFORM_FREE_MEMORY(PlatformFreeMemory);

#define PLATFORM_EXIT_GAME(name) void name()
typedef PLATFORM_EXIT_GAME(PlatformExitGame);

#define PLATFORM_CAPTURE_MOUSE(name) void name()
typedef PLATFORM_CAPTURE_MOUSE(PlatformCaptureMouse);

struct GameMemory
{
    bool isInitialized;

    PlatformExitGame *platformExitGame;
    PlatformCaptureMouse *platformCaptureMouse;
    PlatformQueueAssetLoad *platformQueueAssetLoad;

    GameState state;

    EngineApi *engineApi;
    MemoryArena arena;
};

struct Viewport
{
    uint32 width;
    uint32 height;
};

struct GameInput
{
    uint64 pressedButtons;
    uint64 prevPressedButtons;

    glm::vec2 mouseCursorOffset;
    float mouseScrollOffset;
};

#define BUTTON(name, idx) GAME_INPUT_##name## = 1ULL << idx
enum GameInputButtons : uint64
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
    BUTTON(KEY_LEFT_ALT, 60),
    BUTTON(KEY_DELETE, 61)
};

#define GAME_UPDATE_AND_RENDER(name)                                                                              \
    void name(GameMemory *memory, GameInput *input, Viewport viewport, float deltaTime)
typedef GAME_UPDATE_AND_RENDER(GameUpdateAndRender);

#endif