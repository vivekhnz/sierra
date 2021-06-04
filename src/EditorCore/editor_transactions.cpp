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
    tx->commandBufferSize = 0;

    return tx;
}

void *pushCommandInternal(EditorTransaction *tx, EditorCommandType type, uint64 size)
{
    *((EditorCommandType *)pushTransactionData(tx->queue, sizeof(EditorCommandType))) = type;
    *((uint64 *)pushTransactionData(tx->queue, sizeof(uint64))) = size;
    void *commandData = pushTransactionData(tx->queue, size);
    tx->commandCount++;
    tx->commandBufferSize += sizeof(EditorCommandType) + sizeof(uint64) + size;
    return commandData;
}
#define pushCommand(tx, type)                                                                 \
    (type *)pushCommandInternal(tx, EDITOR_COMMAND_##type, sizeof(type))

struct EditorCommandIterator
{
    uint8 *baseAddress;
    uint32 offset;
    uint32 position;
    uint32 commandCount;
};
struct EditorCommandEntry
{
    EditorCommandType type;
    void *data;
};
bool isIteratorFinished(EditorCommandIterator *iterator)
{
    return iterator->position >= iterator->commandCount;
}
EditorCommandEntry getNextCommand(EditorCommandIterator *iterator)
{
    EditorCommandEntry entry;

    entry.type = *((EditorCommandType *)(iterator->baseAddress + iterator->offset));
    iterator->offset += sizeof(entry.type);

    uint64 commandSize = *((uint64 *)(iterator->baseAddress + iterator->offset));
    iterator->offset += sizeof(commandSize);

    entry.data = iterator->baseAddress + iterator->offset;
    iterator->offset += commandSize;

    iterator->position++;

    return entry;
}

void applyTransactions(EditorTransactionQueue *queue, EditorState *state)
{
    uint8 *baseAddress = (uint8 *)queue->data.baseAddress;
    uint64 offset = 0;
    while (offset < queue->dataStorageUsed)
    {
        EditorTransaction *tx = (EditorTransaction *)(baseAddress + offset);
        offset += sizeof(*tx);

        EditorCommandIterator cmdIterator;
        cmdIterator.baseAddress = baseAddress + offset;
        cmdIterator.offset = 0;
        cmdIterator.position = 0;
        cmdIterator.commandCount = tx->commandCount;

        while (!isIteratorFinished(&cmdIterator))
        {
            EditorCommandEntry cmdEntry = getNextCommand(&cmdIterator);

            switch (cmdEntry.type)
            {
            case EDITOR_COMMAND_AddMaterialCommand:
            {
                AddMaterialCommand *cmd = (AddMaterialCommand *)cmdEntry.data;

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
            }
            break;
            case EDITOR_COMMAND_DeleteMaterialCommand:
            {
                DeleteMaterialCommand *cmd = (DeleteMaterialCommand *)cmdEntry.data;

                assert(cmd->index < MAX_MATERIAL_COUNT);
                state->sceneState.worldState.materialCount--;
                for (uint32 i = cmd->index; i < state->sceneState.worldState.materialCount;
                     i++)
                {
                    state->sceneState.worldState.materialIds[i] =
                        state->sceneState.worldState.materialIds[i + 1];
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
            }
            break;
            case EDITOR_COMMAND_SwapMaterialCommand:
            {
                SwapMaterialCommand *cmd = (SwapMaterialCommand *)cmdEntry.data;

                assert(cmd->indexA < MAX_MATERIAL_COUNT);
                assert(cmd->indexB < MAX_MATERIAL_COUNT);

#define swap(type, array)                                                                     \
    type temp_##array = state->sceneState.worldState.array[cmd->indexA];                      \
    state->sceneState.worldState.array[cmd->indexA] =                                         \
        state->sceneState.worldState.array[cmd->indexB];                                      \
    state->sceneState.worldState.array[cmd->indexB] = temp_##array;

                swap(uint32, materialIds);
                swap(GpuMaterialProperties, materialProps);
                swap(uint32, albedoTextureAssetIds);
                swap(uint32, normalTextureAssetIds);
                swap(uint32, displacementTextureAssetIds);
                swap(uint32, aoTextureAssetIds);
            }
            break;
            case EDITOR_COMMAND_SetMaterialTextureCommand:
            {
                SetMaterialTextureCommand *cmd = (SetMaterialTextureCommand *)cmdEntry.data;

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
                    uint32 *materialTextureAssetIds[] = {
                        state->sceneState.worldState.albedoTextureAssetIds,       //
                        state->sceneState.worldState.normalTextureAssetIds,       //
                        state->sceneState.worldState.displacementTextureAssetIds, //
                        state->sceneState.worldState.aoTextureAssetIds            //
                    };
                    uint32 *textureAssetIds =
                        materialTextureAssetIds[(uint32)cmd->textureType];
                    textureAssetIds[index] = cmd->assetId;
                }
            }
            break;
            case EDITOR_COMMAND_SetMaterialPropertiesCommand:
            {
                SetMaterialPropertiesCommand *cmd =
                    (SetMaterialPropertiesCommand *)cmdEntry.data;

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
            }
            break;
            case EDITOR_COMMAND_AddObjectCommand:
            {
                AddObjectCommand *cmd = (AddObjectCommand *)cmdEntry.data;

                assert(state->sceneState.objectInstanceCount < MAX_OBJECT_INSTANCES);
                uint32 index = state->sceneState.objectInstanceCount++;

                state->sceneState.objectIds[index] = cmd->objectId;

                ObjectTransform *transform = &state->sceneState.objectTransforms[index];
                transform->position = glm::vec3(0);
                transform->rotation = glm::vec3(0);
                transform->scale = glm::vec3(1);
            }
            break;
            case EDITOR_COMMAND_SetObjectTransformCommand:
            {
                SetObjectTransformCommand *cmd = (SetObjectTransformCommand *)cmdEntry.data;

                bool foundObject = false;
                uint32 index = 0;
                for (uint32 i = 0; i < state->sceneState.objectInstanceCount; i++)
                {
                    if (state->sceneState.objectIds[i] == cmd->objectId)
                    {
                        foundObject = true;
                        index = i;
                        break;
                    }
                }

                if (foundObject)
                {
                    ObjectTransform *transform = &state->sceneState.objectTransforms[index];
                    transform->position = cmd->position;
                    transform->rotation = cmd->rotation;
                    transform->scale = cmd->scale;
                }
            }
            break;
            }
        }

        queue->publishTransaction(cmdIterator.baseAddress, tx->commandBufferSize);

        offset += tx->commandBufferSize;
    }

    queue->dataStorageUsed = 0;
}