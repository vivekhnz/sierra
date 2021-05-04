#include "editor_transactions.h"

void *pushTransactionData(EditorTransactionQueue *queue, uint64 size)
{
    uint64 availableStorage = queue->data.size - queue->dataStorageUsed;
    assert(availableStorage >= size);

    void *address = (uint8 *)queue->data.baseAddress + queue->dataStorageUsed;
    queue->dataStorageUsed += size;

    return address;
}

EditorTransaction *transactionsCreate(EditorTransactionQueue *queue)
{
    EditorTransaction *tx =
        (EditorTransaction *)pushTransactionData(queue, sizeof(EditorTransaction));
    tx->queue = queue;
    tx->commandCount = 0;

    return tx;
}

void *transactionsPushCommandInternal(
    EditorTransaction *tx, EditorCommandType type, uint64 size)
{
    *((EditorCommandType *)pushTransactionData(tx->queue, sizeof(EditorCommandType))) = type;
    void *commandData = pushTransactionData(tx->queue, size);
    tx->commandCount++;
    return commandData;
}
#define transactionsPushCommand(tx, type)                                                     \
    (type *)transactionsPushCommandInternal(tx, EDITOR_COMMAND_##type, sizeof(type))

void transactionsApplyChanges(EditorTransactionQueue *queue, EditorState *state)
{
    uint8 *baseAddress = (uint8 *)queue->data.baseAddress;
    uint64 offset = 0;
    while (offset < queue->dataStorageUsed)
    {
        EditorTransaction *tx = (EditorTransaction *)(baseAddress + offset);
        offset += sizeof(*tx);

        for (uint32 i = 0; i < tx->commandCount; i++)
        {
            EditorCommandType commandType = *((EditorCommandType *)(baseAddress + offset));
            offset += sizeof(commandType);
            void *commandData = baseAddress + offset;

            switch (commandType)
            {
            case EDITOR_COMMAND_AddMaterialCommand:
            {
                AddMaterialCommand *cmd = (AddMaterialCommand *)commandData;

                assert(state->docState.materialCount < MAX_MATERIAL_COUNT);
                uint32 index = state->docState.materialCount++;
                TerrainMaterialProperties *material = &state->docState.materialProps[index];
                material->albedoTextureAssetId = cmd->albedoTextureAssetId;
                material->normalTextureAssetId = cmd->normalTextureAssetId;
                material->displacementTextureAssetId = cmd->displacementTextureAssetId;
                material->aoTextureAssetId = cmd->aoTextureAssetId;
                material->textureSizeInWorldUnits = cmd->textureSizeInWorldUnits;
                material->slopeStart = cmd->slopeStart;
                material->slopeEnd = cmd->slopeEnd;
                material->altitudeStart = cmd->altitudeStart;
                material->altitudeEnd = cmd->altitudeEnd;

                offset += sizeof(*cmd);
            }
            break;
            }
        }
    }

    queue->dataStorageUsed = 0;
}