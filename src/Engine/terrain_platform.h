#ifndef TERRAIN_PLATFORM_H
#define TERRAIN_PLATFORM_H

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

struct MemoryBlock
{
    void *baseAddress;
    uint64 size;
};

struct PlatformReadFileResult
{
    uint64 size;
    void *data;
};

struct EngineMemory;

typedef void PlatformAssetLoadCallback(
    EngineMemory *memory, uint32 assetId, PlatformReadFileResult *result);

#define PLATFORM_FREE_MEMORY(name) void name(void *data)
typedef PLATFORM_FREE_MEMORY(PlatformFreeMemory);

#define PLATFORM_READ_FILE(name) PlatformReadFileResult name(const char *path)
typedef PLATFORM_READ_FILE(PlatformReadFile);

#define PLATFORM_LOAD_ASSET(name)                                                             \
    void name(EngineMemory *memory, uint32 assetId, const char *relativePath,                 \
        PlatformAssetLoadCallback onAssetLoaded)
typedef PLATFORM_LOAD_ASSET(PlatformLoadAsset);

struct EngineMemory
{
    void *baseAddress;
    uint64 size;

    PlatformFreeMemory *platformFreeMemory;
    PlatformReadFile *platformReadFile;
    PlatformLoadAsset *platformLoadAsset;

    MemoryBlock renderer;
    MemoryBlock assets;
};

#endif