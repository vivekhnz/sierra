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
    EDITOR_COMMAND_AddMaterialCommand
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

#endif