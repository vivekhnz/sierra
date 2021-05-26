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
        for (uint32 cmdIdx = 0; cmdIdx < tx->commandCount; cmdIdx++)
        {
            EditorCommandType commandType = *((EditorCommandType *)(baseAddress + offset));
            offset += sizeof(commandType);
            void *commandData = baseAddress + offset;

            switch (commandType)
            {
            case EDITOR_COMMAND_AddMaterialCommand:
            {
                AddMaterialCommand *cmd = (AddMaterialCommand *)commandData;

                assert(state->sceneState.worldState.materialCount < MAX_MATERIAL_COUNT);
                uint32 index = state->sceneState.worldState.materialCount++;

                state->sceneState.worldState.materialIds[index] = cmd->materialId;

                GpuMaterialProperties *material =
                    &state->sceneState.worldState.materialProps[index];
                material->textureSizeInWorldUnits.x = cmd->textureSizeInWorldUnits;
                material->textureSizeInWorldUnits.y = cmd->textureSizeInWorldUnits;
                material->rampParams.x = cmd->slopeStart;
                material->rampParams.y = cmd->slopeEnd;
                material->rampParams.z = cmd->altitudeStart;
                material->rampParams.w = cmd->altitudeEnd;

                state->sceneState.worldState.albedoTextureAssetIds[index] =
                    cmd->albedoTextureAssetId;
                state->sceneState.worldState.normalTextureAssetIds[index] =
                    cmd->normalTextureAssetId;
                state->sceneState.worldState.displacementTextureAssetIds[index] =
                    cmd->displacementTextureAssetId;
                state->sceneState.worldState.aoTextureAssetIds[index] = cmd->aoTextureAssetId;

                offset += sizeof(*cmd);
            }
            break;
            case EDITOR_COMMAND_DeleteMaterialCommand:
            {
                DeleteMaterialCommand *cmd = (DeleteMaterialCommand *)commandData;

                assert(cmd->index < MAX_MATERIAL_COUNT);
                state->sceneState.worldState.materialCount--;
                for (uint32 i = cmd->index; i < state->sceneState.worldState.materialCount;
                     i++)
                {
                    state->sceneState.worldState.materialProps[i] =
                        state->sceneState.worldState.materialProps[i + 1];
                    state->sceneState.worldState.albedoTextureAssetIds[i] =
                        state->sceneState.worldState.albedoTextureAssetIds[i + 1];
                    state->sceneState.worldState.normalTextureAssetIds[i] =
                        state->sceneState.worldState.normalTextureAssetIds[i + 1];
                    state->sceneState.worldState.displacementTextureAssetIds[i] =
                        state->sceneState.worldState.displacementTextureAssetIds[i + 1];
                    state->sceneState.worldState.aoTextureAssetIds[i] =
                        state->sceneState.worldState.aoTextureAssetIds[i + 1];
                }

                offset += sizeof(*cmd);
            }
            break;
            case EDITOR_COMMAND_SwapMaterialCommand:
            {
                SwapMaterialCommand *cmd = (SwapMaterialCommand *)commandData;

                assert(cmd->indexA < MAX_MATERIAL_COUNT);
                assert(cmd->indexB < MAX_MATERIAL_COUNT);

#define swap(type, array)                                                                     \
    type temp_##array = state->sceneState.worldState.array[cmd->indexA];                      \
    state->sceneState.worldState.array[cmd->indexA] =                                         \
        state->sceneState.worldState.array[cmd->indexB];                                      \
    state->sceneState.worldState.array[cmd->indexB] = temp_##array;

                swap(GpuMaterialProperties, materialProps);
                swap(uint32, albedoTextureAssetIds);
                swap(uint32, normalTextureAssetIds);
                swap(uint32, displacementTextureAssetIds);
                swap(uint32, aoTextureAssetIds);

                offset += sizeof(*cmd);
            }
            break;
            case EDITOR_COMMAND_SetMaterialTextureCommand:
            {
                SetMaterialTextureCommand *cmd = (SetMaterialTextureCommand *)commandData;

                assert(cmd->index < MAX_MATERIAL_COUNT);

                uint32 *materialTextureAssetIds[] = {
                    state->sceneState.worldState.albedoTextureAssetIds,       //
                    state->sceneState.worldState.normalTextureAssetIds,       //
                    state->sceneState.worldState.displacementTextureAssetIds, //
                    state->sceneState.worldState.aoTextureAssetIds            //
                };
                uint32 *textureAssetIds = materialTextureAssetIds[(uint32)cmd->textureType];
                textureAssetIds[cmd->index] = cmd->assetId;

                offset += sizeof(*cmd);
            }
            break;
            case EDITOR_COMMAND_SetMaterialPropertiesCommand:
            {
                SetMaterialPropertiesCommand *cmd =
                    (SetMaterialPropertiesCommand *)commandData;

                bool foundMaterial = false;
                uint32 index = 0;
                for (uint32 i = 0; i < state->sceneState.worldState.materialCount; i++)
                {
                    if (state->sceneState.worldState.materialIds[i] == cmd->materialId)
                    {
                        foundMaterial = true;
                        index = i;
                        break;
                    }
                }

                if (foundMaterial)
                {
                    GpuMaterialProperties *material =
                        &state->sceneState.worldState.materialProps[index];
                    material->textureSizeInWorldUnits.x = cmd->textureSizeInWorldUnits;
                    material->textureSizeInWorldUnits.y = cmd->textureSizeInWorldUnits;
                    material->rampParams.x = cmd->slopeStart;
                    material->rampParams.y = cmd->slopeEnd;
                    material->rampParams.z = cmd->altitudeStart;
                    material->rampParams.w = cmd->altitudeEnd;
                }

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