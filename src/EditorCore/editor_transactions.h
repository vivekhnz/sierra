#ifndef EDITOR_TRANSACTIONS_H
#define EDITOR_TRANSACTIONS_H

#include "../Engine/engine.h"

struct EditorTransactionQueue
{
    MemoryBlock data;
    uint64 dataStorageUsed;
};

enum EditorCommandType
{
    EDITOR_COMMAND_AddMaterialCommand,
    EDITOR_COMMAND_DeleteMaterialCommand,
    EDITOR_COMMAND_SwapMaterialCommand,
    EDITOR_COMMAND_SetMaterialTextureCommand,
    EDITOR_COMMAND_SetMaterialPropertiesCommand,
    EDITOR_COMMAND_AddObjectCommand,
    EDITOR_COMMAND_SetObjectTransformCommand
};

struct EditorTransaction
{
    EditorTransactionQueue *queue;
    uint32 commandBufferSize;
};

struct AddMaterialCommand
{
    uint32 materialId;

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

struct DeleteMaterialCommand
{
    uint32 index;
};

struct SwapMaterialCommand
{
    uint32 indexA;
    uint32 indexB;
};

enum TerrainMaterialTextureType
{
    TERRAIN_MAT_TEXTURE_ALBEDO = 0,
    TERRAIN_MAT_TEXTURE_NORMAL = 1,
    TERRAIN_MAT_TEXTURE_DISPLACEMENT = 2,
    TERRAIN_MAT_TEXTURE_AMBIENT_OCCLUSION = 3
};
struct SetMaterialTextureCommand
{
    uint32 materialId;
    TerrainMaterialTextureType textureType;
    uint32 assetId;
};

struct SetMaterialPropertiesCommand
{
    uint32 materialId;
    float textureSizeInWorldUnits;
    float slopeStart;
    float slopeEnd;
    float altitudeStart;
    float altitudeEnd;
};

struct AddObjectCommand
{
    uint32 objectId;
};

struct SetObjectTransformCommand
{
    uint32 objectId;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

#endif