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
    EDITOR_COMMAND_SetObjectTransformCommand
};

enum ActiveTransactionType
{
    ACTIVE_TX_MOVE_OBJECT = 0,

    ACTIVE_TX_COUNT
};

struct CommandBuffer
{
    void *baseAddress;
    uint64 size;
    uint64 used;
};

struct TransactionState;
struct ActiveTransactionDataBlock
{
    TransactionState *transactions;
    ActiveTransactionType type;
    CommandBuffer commandBuffer;
    ActiveTransactionDataBlock *prev;
    ActiveTransactionDataBlock *next;
};

struct TransactionState
{
    ActiveTransactionDataBlock activeData[MAX_CONCURRENT_ACTIVE_TRANSACTIONS];
    ActiveTransactionDataBlock *firstActive;
    ActiveTransactionDataBlock *activeByType[ACTIVE_TX_COUNT];
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

struct SetObjectTransformCommand
{
    uint32 objectId;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

#endif