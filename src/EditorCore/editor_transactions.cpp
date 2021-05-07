#include "editor_transactions.h"

void *pushTransactionData(EditorTransactionQueue *queue, uint64 size)
{
    uint64 availableStorage = queue->data.size - queue->dataStorageUsed;
    assert(availableStorage >= size);

    void *address = (uint8 *)queue->data.baseAddress + queue->dataStorageUsed;
    queue->dataStorageUsed += size;

    return address;
}

EditorTransaction *createTransaction(EditorTransactionQueue *queue)
{
    EditorTransaction *tx =
        (EditorTransaction *)pushTransactionData(queue, sizeof(EditorTransaction));
    tx->queue = queue;
    tx->commandCount = 0;

    return tx;
}

void *pushCommandInternal(EditorTransaction *tx, EditorCommandType type, uint64 size)
{
    *((EditorCommandType *)pushTransactionData(tx->queue, sizeof(EditorCommandType))) = type;
    void *commandData = pushTransactionData(tx->queue, size);
    tx->commandCount++;
    return commandData;
}
#define pushCommand(tx, type)                                                                 \
    (type *)pushCommandInternal(tx, EDITOR_COMMAND_##type, sizeof(type))

void applyTransactions(EditorTransactionQueue *queue, EditorState *state)
{
    uint8 *baseAddress = (uint8 *)queue->data.baseAddress;
    uint64 offset = 0;
    while (offset < queue->dataStorageUsed)
    {
        EditorTransaction *tx = (EditorTransaction *)(baseAddress + offset);
        offset += sizeof(*tx);

        uint8 *commandBufferBaseAddress = baseAddress + offset;
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
            case EDITOR_COMMAND_DeleteMaterialCommand:
            {
                DeleteMaterialCommand *cmd = (DeleteMaterialCommand *)commandData;

                assert(cmd->index < MAX_MATERIAL_COUNT);
                state->docState.materialCount--;
                for (uint32 i = cmd->index; i < state->docState.materialCount; i++)
                {
                    state->docState.materialProps[i] = state->docState.materialProps[i + 1];
                }

                offset += sizeof(*cmd);
            }
            break;
            case EDITOR_COMMAND_SwapMaterialCommand:
            {
                SwapMaterialCommand *cmd = (SwapMaterialCommand *)commandData;

                assert(cmd->indexA < MAX_MATERIAL_COUNT);
                assert(cmd->indexB < MAX_MATERIAL_COUNT);

                TerrainMaterialProperties temp = state->docState.materialProps[cmd->indexA];
                state->docState.materialProps[cmd->indexA] =
                    state->docState.materialProps[cmd->indexB];
                state->docState.materialProps[cmd->indexB] = temp;

                offset += sizeof(*cmd);
            }
            break;
            case EDITOR_COMMAND_SetMaterialTextureCommand:
            {
                SetMaterialTextureCommand *cmd = (SetMaterialTextureCommand *)commandData;

                assert(cmd->index < MAX_MATERIAL_COUNT);
                TerrainMaterialProperties *matProps =
                    &state->docState.materialProps[cmd->index];
                uint32 *materialTextureAssetIds[] = {
                    &matProps->albedoTextureAssetId,       //
                    &matProps->normalTextureAssetId,       //
                    &matProps->displacementTextureAssetId, //
                    &matProps->aoTextureAssetId            //
                };
                *materialTextureAssetIds[(uint32)cmd->textureType] = cmd->assetId;

                offset += sizeof(*cmd);
            }
            break;
            case EDITOR_COMMAND_SetMaterialPropertiesCommand:
            {
                SetMaterialPropertiesCommand *cmd =
                    (SetMaterialPropertiesCommand *)commandData;

                assert(cmd->index < MAX_MATERIAL_COUNT);
                TerrainMaterialProperties *matProps =
                    &state->docState.materialProps[cmd->index];
                matProps->textureSizeInWorldUnits = cmd->textureSizeInWorldUnits;
                matProps->slopeStart = cmd->slopeStart;
                matProps->slopeEnd = cmd->slopeEnd;
                matProps->altitudeStart = cmd->altitudeStart;
                matProps->altitudeEnd = cmd->altitudeEnd;

                offset += sizeof(*cmd);
            }
            break;
            }
        }

        uint64 commandBufferSize = (baseAddress + offset) - commandBufferBaseAddress;
        queue->publishTransaction(commandBufferBaseAddress, commandBufferSize);
    }

    queue->dataStorageUsed = 0;
}