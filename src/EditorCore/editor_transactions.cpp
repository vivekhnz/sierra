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

void clearTransaction(Transaction *tx)
{
    // the first 8 bytes of the command buffer is the no. of bytes used within the buffer
    // this is inclusive of the 8 bytes storing the amount used
    *((uint64 *)tx->commandBufferBaseAddress) = sizeof(uint64);
}

Transaction *beginTransaction(TransactionState *state)
{
    TransactionDataBlock *result = 0;
    if (state->nextFreeActive)
    {
        result = state->nextFreeActive;
        clearTransaction(&result->tx);

        state->nextFreeActive = state->nextFreeActive->prev;

        if (state->firstActive)
        {
            TransactionDataBlock *lastTx = state->firstActive;
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

void *pushCommandInternal(Transaction *tx, EditorCommandType type, uint64 size)
{
    *((EditorCommandType *)pushTransactionData(tx, sizeof(EditorCommandType))) = type;
    *((uint64 *)pushTransactionData(tx, sizeof(uint64))) = size;
    return pushTransactionData(tx, size);
}
#define pushCommand(tx, type)                                                                 \
    (type *)pushCommandInternal(tx, EDITOR_COMMAND_##type, sizeof(type))

void discardTransaction(Transaction *tx)
{
    TransactionDataBlock *block = (TransactionDataBlock *)tx;
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

void commitTransaction(Transaction *tx)
{
    TransactionDataBlock *block = (TransactionDataBlock *)tx;
    uint64 *srcCommandBufferSize = (uint64 *)tx->commandBufferBaseAddress;

    TransactionState *transactions = block->transactions;
    uint64 availableStorage = transactions->committedSize - transactions->committedUsed;
    assert(availableStorage >= *srcCommandBufferSize);

    void *dst = (uint8 *)transactions->committedBaseAddress + transactions->committedUsed;
    transactions->committedUsed += *srcCommandBufferSize;

    memcpy(dst, tx->commandBufferBaseAddress, *srcCommandBufferSize);
    discardTransaction(tx);
}

bool isTransactionValid(TransactionEntry *tx)
{
    return tx->commandBufferBaseAddress;
}

TransactionEntry getFirstCommittedTransaction(TransactionState *transactions)
{
    TransactionEntry result = {};
    result.owner = transactions;
    result.commandBufferBaseAddress = 0;

    if (transactions->committedUsed > 0)
    {
        result.commandBufferBaseAddress = transactions->committedBaseAddress;
    }

    return result;
}
TransactionEntry getNextCommittedTransaction(TransactionEntry *tx)
{
    TransactionEntry result = {};
    result.commandBufferBaseAddress = 0;

    TransactionState *transactions = (TransactionState *)tx->owner;

    uint64 commandBufferSize = *((uint64 *)tx->commandBufferBaseAddress);
    void *nextCommandBufferBaseAddress =
        (uint8 *)tx->commandBufferBaseAddress + commandBufferSize;
    void *endOfCommittedTransactions =
        (uint8 *)transactions->committedBaseAddress + transactions->committedUsed;

    if (nextCommandBufferBaseAddress < endOfCommittedTransactions)
    {
        result.owner = tx->owner;
        result.commandBufferBaseAddress = nextCommandBufferBaseAddress;
    }

    return result;
}

TransactionEntry getFirstActiveTransaction(TransactionState *transactions)
{
    TransactionEntry result = {};
    result.owner = 0;
    result.commandBufferBaseAddress = 0;

    if (transactions->firstActive)
    {
        result.owner = transactions->firstActive;
        result.commandBufferBaseAddress =
            transactions->firstActive->tx.commandBufferBaseAddress;
    }

    return result;
}
TransactionEntry getNextActiveTransaction(TransactionEntry *tx)
{
    TransactionEntry result = {};
    result.commandBufferBaseAddress = 0;

    TransactionDataBlock *block = (TransactionDataBlock *)tx->owner;
    if (block->next)
    {
        result.owner = block->next;
        result.commandBufferBaseAddress = block->next->tx.commandBufferBaseAddress;
    }

    return result;
}

CommandEntry getFirstCommand(TransactionEntry *tx)
{
    CommandEntry result = {};
    result.data = 0;
    result.size = 0;

    uint8 *position = (uint8 *)tx->commandBufferBaseAddress;

    uint64 commandBufferSize = *((uint64 *)position);
    position += sizeof(uint64);

    if (commandBufferSize > sizeof(uint64))
    {
        result.type = *((EditorCommandType *)position);
        position += sizeof(result.type);

        result.size = *((uint64 *)position);
        position += sizeof(result.size);

        result.data = position;
    }

    return result;
}
bool isCommandValid(CommandEntry *cmdEntry)
{
    return cmdEntry->data;
}
CommandEntry getNextCommand(TransactionEntry *tx, CommandEntry *cmdEntry)
{
    CommandEntry result = {};
    result.data = 0;

    uint64 commandBufferSize = *((uint64 *)tx->commandBufferBaseAddress);
    void *nextCommandAddress = (uint8 *)cmdEntry->data + cmdEntry->size;
    void *endOfCommandBuffer = (uint8 *)tx->commandBufferBaseAddress + commandBufferSize;

    if (nextCommandAddress < endOfCommandBuffer)
    {
        uint8 *position = (uint8 *)nextCommandAddress;

        result.type = *((EditorCommandType *)position);
        position += sizeof(result.type);

        result.size = *((uint64 *)position);
        position += sizeof(result.size);

        result.data = position;
    }

    return result;
}