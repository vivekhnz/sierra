#include "editor_transactions.h"

void *pushTransactionData(Transaction *tx, uint64 size)
{
    uint64 *used = (uint64 *)tx->commandBufferBaseAddress;

    uint64 availableStorage = tx->commandBufferMaxSize - *used;
    assert(availableStorage >= size);

    void *address = (uint8 *)tx->commandBufferBaseAddress + *used;
    *used += size;

    return address;
}

Transaction *beginTransaction(TransactionState *state)
{
    assert(!state->isInTransaction);
    state->isInTransaction = true;

    uint8 *baseAddress = (uint8 *)state->committedBaseAddress + state->committedUsed;
    Transaction *tx = (Transaction *)baseAddress;
    tx->transactions = state;
    tx->block = 0;
    tx->commandBufferBaseAddress = baseAddress + sizeof(Transaction);
    tx->commandBufferMaxSize =
        state->committedSize - (state->committedUsed + sizeof(Transaction));

    // the first 8 bytes of the command buffer is the no. of bytes used within the buffer
    // this is inclusive of the 8 bytes storing the amount used
    *((uint64 *)tx->commandBufferBaseAddress) = sizeof(uint64);

    return tx;
}
void endTransaction(Transaction *tx)
{
    assert(tx->transactions->isInTransaction);
    tx->transactions->isInTransaction = false;

    uint64 *used = (uint64 *)tx->commandBufferBaseAddress;
    tx->commandBufferMaxSize = *used;
    tx->transactions->committedUsed += sizeof(Transaction) + tx->commandBufferMaxSize;
}

Transaction *beginActiveTransaction(TransactionState *state)
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

    return result ? &result->tx : 0;
}
void discardActiveTransaction(Transaction *tx)
{
    assert(tx->block);
    ActiveTransactionDataBlock *block = tx->block;
    if (block->prev)
    {
        block->prev->next = block->next;
    }
    else
    {
        block->transactions->firstActive = block->next;
    }
    if (block->next)
    {
        block->next->prev = block->prev;
    }
    block->next = 0;
    block->prev = block->transactions->nextFreeActive;
    block->transactions->nextFreeActive = block;
}
void commitActiveTransaction(Transaction *activeTx)
{
    assert(activeTx->block);
    uint64 *srcBufferUsed = (uint64 *)activeTx->commandBufferBaseAddress;
    uint64 usedExcludingSize = *srcBufferUsed - sizeof(uint64);
    uint8 *srcBufferCommands = (uint8 *)activeTx->commandBufferBaseAddress + sizeof(uint64);

    Transaction *commitTx = beginTransaction(activeTx->block->transactions);

    void *dst = pushTransactionData(commitTx, usedExcludingSize);
    memcpy(dst, srcBufferCommands, usedExcludingSize);

    endTransaction(commitTx);
    discardActiveTransaction(activeTx);
}

void *pushCommandInternal(Transaction *tx, EditorCommandType type, uint64 size)
{
    *((EditorCommandType *)pushTransactionData(tx, sizeof(EditorCommandType))) = type;
    *((uint64 *)pushTransactionData(tx, sizeof(uint64))) = size;
    return pushTransactionData(tx, size);
}
#define pushCommand(tx, type)                                                                 \
    (type *)pushCommandInternal(tx, EDITOR_COMMAND_##type, sizeof(type))

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
Iterator getIterator(Transaction *tx)
{
    uint64 *used = (uint64 *)tx->commandBufferBaseAddress;

    Iterator iterator;
    iterator.position = (uint8 *)tx->commandBufferBaseAddress + sizeof(uint64);
    iterator.end = (uint8 *)tx->commandBufferBaseAddress + *used;
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
Transaction *getNextTransaction(Iterator *iterator)
{
    Transaction *tx = (Transaction *)iterator->position;
    iterator->position += sizeof(*tx) + tx->commandBufferMaxSize;
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