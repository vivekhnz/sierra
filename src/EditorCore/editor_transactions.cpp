#include "editor_transactions.h"

void *pushCommandBufferData(CommandBuffer *buffer, uint64 size)
{
    uint64 availableStorage = buffer->size - buffer->used;
    assert(availableStorage >= size);

    void *address = (uint8 *)buffer->baseAddress + buffer->used;
    buffer->used += size;

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
    tx->commandBuffer.used = 0;
    tx->commandBuffer.size =
        state->committedSize - (state->committedUsed + sizeof(EditorTransaction));

    return tx;
}
void endTransaction(EditorTransaction *tx)
{
    assert(tx->state->isInTransaction);
    tx->state->isInTransaction = false;

    tx->commandBuffer.size = tx->commandBuffer.used;
    tx->state->committedUsed += sizeof(EditorTransaction) + tx->commandBuffer.size;
}

ActiveTransactionDataBlock *beginActiveTransaction(
    TransactionState *state, ActiveTransactionType type)
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
        result->type = type;
        state->activeByType[type] = result;
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
    tx->transactions->activeByType[tx->type] = 0;
}
void commitActiveTransaction(ActiveTransactionDataBlock *activeTx)
{
    EditorTransaction *commitTx = beginTransaction(activeTx->transactions);

    CommandBuffer *srcBuffer = &activeTx->commandBuffer;
    CommandBuffer *dstBuffer = &commitTx->commandBuffer;
    void *dst = pushCommandBufferData(dstBuffer, srcBuffer->used);
    memcpy(dst, srcBuffer->baseAddress, srcBuffer->used);

    endTransaction(commitTx);
    discardActiveTransaction(activeTx);
}
ActiveTransactionDataBlock *getActiveTransaction(
    TransactionState *state, ActiveTransactionType type)
{
    return state->activeByType[type];
}

void *pushCommandInternal(CommandBuffer *buffer, EditorCommandType type, uint64 size)
{
    *((EditorCommandType *)pushCommandBufferData(buffer, sizeof(EditorCommandType))) = type;
    *((uint64 *)pushCommandBufferData(buffer, sizeof(uint64))) = size;
    return pushCommandBufferData(buffer, size);
}
#define pushCommand(tx, type)                                                                 \
    (type *)pushCommandInternal(&tx->commandBuffer, EDITOR_COMMAND_##type, sizeof(type))

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
    CommandBuffer commandBuffer;
    EditorCommandIterator commandIterator;
};
EditorTransactionIterator getIterator(TransactionState *state)
{
    EditorTransactionIterator iterator;
    iterator.position = (uint8 *)state->committedBaseAddress;
    iterator.end = iterator.position + state->committedUsed;

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

    entry.commandBuffer.baseAddress = iterator->position;
    entry.commandBuffer.size = tx->commandBuffer.used;
    entry.commandBuffer.used = tx->commandBuffer.used;
    entry.commandIterator.position = (uint8 *)entry.commandBuffer.baseAddress;
    entry.commandIterator.end =
        (uint8 *)entry.commandBuffer.baseAddress + entry.commandBuffer.size;

    iterator->position = entry.commandIterator.end;

    return entry;
}