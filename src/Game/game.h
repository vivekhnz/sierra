#ifndef GAME_H
#define GAME_H

#include <glm/gtc/type_ptr.hpp>

#include "../Engine/terrain_platform.h"
#include "../Engine/terrain_heightfield.h"
#include "../Engine/IO/InputManager.hpp"

#define HEIGHTFIELD_ROWS 256
#define HEIGHTFIELD_COLUMNS 256
#define MATERIAL_COUNT 3

struct GameState
{
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
    float heightfieldHeights[HEIGHTFIELD_ROWS * HEIGHTFIELD_COLUMNS];

    uint32 terrainMeshElementCount;
    uint32 terrainMeshVertexBufferHandle;
    uint32 terrainMeshVertexArrayHandle;
    uint32 terrainMeshTessLevelBufferHandle;

    uint32 heightmapTextureHandle;
    uint32 albedoTextureArrayHandle;
    uint32 normalTextureArrayHandle;
    uint32 displacementTextureArrayHandle;
    uint32 aoTextureArrayHandle;

    uint8 groundAlbedoTextureVersion;
    uint8 rockAlbedoTextureVersion;
    uint8 snowAlbedoTextureVersion;
    uint8 groundNormalTextureVersion;
    uint8 rockNormalTextureVersion;
    uint8 snowNormalTextureVersion;
    uint8 groundDisplacementTextureVersion;
    uint8 rockDisplacementTextureVersion;
    uint8 snowDisplacementTextureVersion;
    uint8 groundAoTextureVersion;
    uint8 rockAoTextureVersion;
    uint8 snowAoTextureVersion;

    uint32 materialPropsBufferHandle;
};

struct GameMemory
{
    bool isInitialized;

    EngineMemory *engine;
    Terrain::Engine::IO::InputManager *input;

    GameState state;
};

struct Viewport
{
    uint32 width;
    uint32 height;
};

void gameUpdateAndRender(GameMemory *memory, Viewport viewport, float deltaTime);
void gameShutdown(GameMemory *memory);

#endif