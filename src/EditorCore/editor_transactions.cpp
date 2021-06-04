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
    tx->commandBufferSize = 0;

    return tx;
}

void *pushCommandInternal(EditorTransaction *tx, EditorCommandType type, uint64 size)
{
    *((EditorCommandType *)pushTransactionData(tx->queue, sizeof(EditorCommandType))) = type;
    *((uint64 *)pushTransactionData(tx->queue, sizeof(uint64))) = size;
    void *commandData = pushTransactionData(tx->queue, size);
    tx->commandBufferSize += sizeof(EditorCommandType) + sizeof(uint64) + size;
    return commandData;
}
#define pushCommand(tx, type)                                                                 \
    (type *)pushCommandInternal(tx, EDITOR_COMMAND_##type, sizeof(type))

struct EditorCommandIterator
{
    uint8 *position;
    uint8 *end;
};
struct EditorCommandEntry
{
    EditorCommandType type;
    void *data;
};
bool isIteratorFinished(EditorCommandIterator *iterator)
{
    return iterator->position >= iterator->end;
}
EditorCommandEntry getNextCommand(EditorCommandIterator *iterator)
{
    EditorCommandEntry entry;

    entry.type = *((EditorCommandType *)iterator->position);
    iterator->position += sizeof(entry.type);

    uint64 commandSize = *((uint64 *)iterator->position);
    iterator->position += sizeof(commandSize);

    entry.data = iterator->position;
    iterator->position += commandSize;

    return entry;
}

struct EditorTransactionIterator
{
    uint8 *position;
    uint8 *end;
};
struct EditorTransactionEntry
{
    struct
    {
        uint8 *start;
        uint32 size;
        EditorCommandIterator iterator;
    } commandBuffer;
};
EditorTransactionIterator getIterator(EditorTransactionQueue *queue)
{
    EditorTransactionIterator iterator;
    iterator.position = (uint8 *)queue->data.baseAddress;
    iterator.end = iterator.position + queue->dataStorageUsed;

    return iterator;
}
bool isIteratorFinished(EditorTransactionIterator *iterator)
{
    return iterator->position >= iterator->end;
}
EditorTransactionEntry getNextTransaction(EditorTransactionIterator *iterator)
{
    EditorTransactionEntry entry;

    EditorTransaction *tx = (EditorTransaction *)iterator->position;
    iterator->position += sizeof(*tx);

    entry.commandBuffer.start = iterator->position;
    entry.commandBuffer.size = tx->commandBufferSize;
    entry.commandBuffer.iterator.position = entry.commandBuffer.start;
    entry.commandBuffer.iterator.end = entry.commandBuffer.start + entry.commandBuffer.size;

    iterator->position = entry.commandBuffer.iterator.end;

    return entry;
}