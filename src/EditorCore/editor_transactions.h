#ifndef EDITOR_TRANSACTIONS_H
#define EDITOR_TRANSACTIONS_H

#include "../Engine/engine.h"

#define PLATFORM_PUBLISH_TRANSACTION(name)                                                    \
    void name(void *commandBufferBaseAddress, uint64 commandBufferSize)
typedef PLATFORM_PUBLISH_TRANSACTION(PlatformPublishTransaction);

struct EditorTransactionQueue
{
    MemoryBlock data;
    uint64 dataStorageUsed;
    PlatformPublishTransaction *publishTransaction;
};

enum EditorCommandType
{
    EDITOR_COMMAND_AddMaterialCommand,
    EDITOR_COMMAND_DeleteMaterialCommand,
    EDITOR_COMMAND_SwapMaterialCommand,
    EDITOR_COMMAND_SetMaterialTextureCommand,
    EDITOR_COMMAND_SetMaterialPropertiesCommand
};

struct EditorTransaction
{
    EditorTransactionQueue *queue;
    uint32 commandCount;
};

struct AddMaterialCommand
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
    uint32 index;
    TerrainMaterialTextureType textureType;
    uint32 assetId;
};

struct SetMaterialPropertiesCommand
{
    uint32 index;
    float textureSizeInWorldUnits;
    float slopeStart;
    float slopeEnd;
    float altitudeStart;
    float altitudeEnd;
};

#endif