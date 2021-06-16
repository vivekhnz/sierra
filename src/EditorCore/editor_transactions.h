#ifndef EDITOR_TRANSACTIONS_H
#define EDITOR_TRANSACTIONS_H

#include "../Engine/engine.h"

#define MAX_CONCURRENT_ACTIVE_TRANSACTIONS 4

enum EditorCommandType
{
    EDITOR_COMMAND_AddMaterialCommand,
    EDITOR_COMMAND_DeleteMaterialCommand,
    EDITOR_COMMAND_SwapMaterialCommand,
    EDITOR_COMMAND_SetMaterialTextureCommand,
    EDITOR_COMMAND_SetMaterialPropertiesCommand,
    EDITOR_COMMAND_AddObjectCommand,
    EDITOR_COMMAND_SetObjectPropertyCommand
};

struct CommandBuffer
{
    void *baseAddress;
    uint64 size;
};

struct TransactionState;
struct ActiveTransactionDataBlock
{
    TransactionState *transactions;
    CommandBuffer commandBuffer;
    ActiveTransactionDataBlock *prev;
    ActiveTransactionDataBlock *next;
};

struct TransactionState
{
    ActiveTransactionDataBlock activeData[MAX_CONCURRENT_ACTIVE_TRANSACTIONS];
    ActiveTransactionDataBlock *firstActive;
    ActiveTransactionDataBlock *nextFreeActive;

    void *committedBaseAddress;
    uint64 committedSize;
    uint64 committedUsed;

    bool isInTransaction;
};

struct EditorTransaction
{
    TransactionState *state;
    CommandBuffer commandBuffer;
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

enum ObjectProperty
{
    PROP_OBJ_POSITION_X,
    PROP_OBJ_POSITION_Y,
    PROP_OBJ_POSITION_Z,
    PROP_OBJ_ROTATION_X,
    PROP_OBJ_ROTATION_Y,
    PROP_OBJ_ROTATION_Z,
    PROP_OBJ_SCALE_X,
    PROP_OBJ_SCALE_Y,
    PROP_OBJ_SCALE_Z,
};
struct SetObjectPropertyCommand
{
    uint32 objectId;
    ObjectProperty property;
    float value;
};

#endif