#ifndef TERRAIN_FOUNDATION_H
#define TERRAIN_FOUNDATION_H

#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define EXPORT __declspec(dllexport)

#if _DEBUG
#define assert(expr)                                                                          \
    if (!(expr))                                                                              \
        *(int *)0 = 0;
#else
#define assert(expr)
#endif

EXPORT struct MemoryBlock
{
    void *baseAddress;
    uint64 size;
};

struct EngineMemory
{
    MemoryBlock renderer;
    MemoryBlock assets;
};

#endif