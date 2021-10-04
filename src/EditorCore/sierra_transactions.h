#ifndef SIERRA_TRANSACTIONS_H
#define SIERRA_TRANSACTIONS_H

#define MAX_CONCURRENT_TRANSACTIONS 4

enum EditorCommandType
{
    EDITOR_COMMAND_AddMaterialCommand,
    EDITOR_COMMAND_DeleteMaterialCommand,
    EDITOR_COMMAND_SwapMaterialCommand,
    EDITOR_COMMAND_SetMaterialTextureCommand,
    EDITOR_COMMAND_SetMaterialPropertiesCommand,
    EDITOR_COMMAND_AddObjectCommand,
    EDITOR_COMMAND_DeleteObjectCommand,
    EDITOR_COMMAND_SetObjectPropertyCommand
};

struct Transaction
{
    void *commandBufferBaseAddress;
    uint64 commandBufferMaxSize;
};

struct TransactionState;
struct TransactionDataBlock
{
    // the Transaction should be the first element of the struct so we can cast between
    // Transaction* and TransactionDataBlock*
    Transaction tx;

    TransactionState *transactions;
    TransactionDataBlock *prev;
    TransactionDataBlock *next;
};

struct TransactionState
{
    TransactionDataBlock txData[MAX_CONCURRENT_TRANSACTIONS];
    TransactionDataBlock *firstActive;
    TransactionDataBlock *nextFreeActive;

    void *committedBaseAddress;
    uint64 committedSize;
    uint64 committedUsed;
};

struct TransactionEntry
{
    void *commandBufferBaseAddress;
    void *owner;
};
struct CommandEntry
{
    EditorCommandType type;
    void *data;
    uint64 size;
};

struct AddMaterialCommand
{
    uint32 materialId;

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
    AssetHandle assetHandle;
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

struct DeleteObjectCommand
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