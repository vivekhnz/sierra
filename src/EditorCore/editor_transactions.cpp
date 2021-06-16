#include "editor_transactions.h"

void *pushCommandBufferData(CommandBuffer *buffer, uint64 size)
{
    uint64 *used = (uint64 *)buffer->baseAddress;

    uint64 availableStorage = buffer->size - *used;
    assert(availableStorage >= size);

    void *address = (uint8 *)buffer->baseAddress + *used;
    *used += size;

    return address;
}

EditorTransaction *beginTransaction(TransactionState *state)
{
    assert(!state->isInTransaction);
    state->isInTransaction = true;

    uint8 *baseAddress = (uint8 *)state->committedBaseAddress + state->committedUsed;
    EditorTransaction *tx = (EditorTransaction *)baseAddress;
    tx->state = state;
    tx->commandBuffer.baseAddress = baseAddress + sizeof(EditorTransaction);
    tx->commandBuffer.size =
        state->committedSize - (state->committedUsed + sizeof(EditorTransaction));

    // the first 8 bytes of the command buffer is the no. of bytes used within the buffer
    // this is inclusive of the 8 bytes storing the amount used
    *((uint64 *)tx->commandBuffer.baseAddress) = sizeof(uint64);

    return tx;
}
void endTransaction(EditorTransaction *tx)
{
    assert(tx->state->isInTransaction);
    tx->state->isInTransaction = false;

    uint64 *used = (uint64 *)tx->commandBuffer.baseAddress;
    tx->commandBuffer.size = *used;
    tx->state->committedUsed += sizeof(EditorTransaction) + tx->commandBuffer.size;
}

ActiveTransactionDataBlock *beginActiveTransaction(TransactionState *state)
{
    ActiveTransactionDataBlock *result = 0;
    if (state->nextFreeActive)
    {
        result = state->nextFreeActive;
        state->nextFreeActive = state->nextFreeActive->prev;

        if (state->firstActive)
        {
            ActiveTransactionDataBlock *lastTx = state->firstActive;
            while (lastTx->next)
            {
                lastTx = lastTx->next;
            }
            result->prev = lastTx;
            lastTx->next = result;
        }
        else
        {
            result->prev = 0;
            state->firstActive = result;
        }
    }

    return result;
}
void discardActiveTransaction(ActiveTransactionDataBlock *tx)
{
    if (tx->prev)
    {
        tx->prev->next = tx->next;
    }
    else
    {
        tx->transactions->firstActive = tx->next;
    }
    if (tx->next)
    {
        tx->next->prev = tx->prev;
    }
    tx->next = 0;
    tx->prev = tx->transactions->nextFreeActive;
    tx->transactions->nextFreeActive = tx;
}
void commitActiveTransaction(ActiveTransactionDataBlock *activeTx)
{
    CommandBuffer *srcBuffer = &activeTx->commandBuffer;
    uint64 *srcBufferUsed = (uint64 *)srcBuffer->baseAddress;
    uint64 usedExcludingSize = *srcBufferUsed - sizeof(uint64);
    uint8 *srcBufferCommands = (uint8 *)srcBuffer->baseAddress + sizeof(uint64);

    EditorTransaction *commitTx = beginTransaction(activeTx->transactions);
    CommandBuffer *dstBuffer = &commitTx->commandBuffer;

    void *dst = pushCommandBufferData(dstBuffer, usedExcludingSize);
    memcpy(dst, srcBufferCommands, usedExcludingSize);

    endTransaction(commitTx);
    discardActiveTransaction(activeTx);
}

void *pushCommandInternal(CommandBuffer *buffer, EditorCommandType type, uint64 size)
{
    *((EditorCommandType *)pushCommandBufferData(buffer, sizeof(EditorCommandType))) = type;
    *((uint64 *)pushCommandBufferData(buffer, sizeof(uint64))) = size;
    return pushCommandBufferData(buffer, size);
}
#define pushCommand(tx, type)                                                                 \
    (type *)pushCommandInternal(&tx->commandBuffer, EDITOR_COMMAND_##type, sizeof(type))

struct Iterator
{
    uint8 *position;
    uint8 *end;
};
struct CommandEntry
{
    EditorCommandType type;
    void *data;
};
Iterator getIterator(CommandBuffer *commandBuffer)
{
    uint64 *used = (uint64 *)commandBuffer->baseAddress;

    Iterator iterator;
    iterator.position = (uint8 *)commandBuffer->baseAddress + sizeof(uint64);
    iterator.end = (uint8 *)commandBuffer->baseAddress + *used;
    return iterator;
}
Iterator getIterator(TransactionState *state)
{
    Iterator iterator;
    iterator.position = (uint8 *)state->committedBaseAddress;
    iterator.end = iterator.position + state->committedUsed;
    return iterator;
}
bool isIteratorFinished(Iterator *iterator)
{
    return iterator->position >= iterator->end;
}
EditorTransaction *getNextTransaction(Iterator *iterator)
{
    EditorTransaction *tx = (EditorTransaction *)iterator->position;
    iterator->position += sizeof(*tx) + tx->commandBuffer.size;
    return tx;
}
CommandEntry getNextCommand(Iterator *iterator)
{
    CommandEntry entry;

    entry.type = *((EditorCommandType *)iterator->position);
    iterator->position += sizeof(entry.type);

    uint64 commandSize = *((uint64 *)iterator->position);
    iterator->position += sizeof(commandSize);

    entry.data = iterator->position;
    iterator->position += commandSize;

    return entry;
}