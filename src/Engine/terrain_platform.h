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

#define global_variable static

#define EXPORT __declspec(dllexport)
#define API_EXPORT extern "C" __declspec(dllexport)

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

struct EngineMemory;

#define PLATFORM_ASSET_LOAD_CALLBACK(name)                                                    \
    void name(EngineMemory *memory, uint32 assetId, void *data, uint64 size)
typedef PLATFORM_ASSET_LOAD_CALLBACK(PlatformAssetLoadCallback);

#define PLATFORM_LOG_MESSAGE(name) void name(char *message)
typedef PLATFORM_LOG_MESSAGE(PlatformLogMessage);

#define PLATFORM_LOAD_ASSET(name)                                                             \
    bool name(EngineMemory *memory, uint32 assetId, const char *relativePath,                 \
        PlatformAssetLoadCallback onAssetLoaded)
typedef PLATFORM_LOAD_ASSET(PlatformLoadAsset);

struct EngineMemory
{
    void *baseAddress;
    uint64 size;

    PlatformLogMessage *platformLogMessage;
    PlatformLoadAsset *platformLoadAsset;

    MemoryBlock renderer;
    MemoryBlock assets;
};

#endif