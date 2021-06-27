#ifndef ENGINE_PLATFORM_H
#define ENGINE_PLATFORM_H

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

struct MemoryArena
{
    void *baseAddress;
    uint64 size;
    uint64 used;
};
inline void *pushSize(MemoryArena *arena, uint64 size)
{
    uint64 availableStorage = arena->size - arena->used;
    assert(availableStorage >= size);

    void *address = (uint8 *)arena->baseAddress + arena->used;
    arena->used += size;

    return address;
}
#define pushStruct(arena, struct) (struct *)pushSize(arena, sizeof(struct))

inline MemoryArena pushSubArena(MemoryArena *arena, uint64 size)
{
    MemoryArena result = {};
    result.size = size;
    result.baseAddress = pushSize(arena, result.size);
    result.used = 0;

    return result;
}

struct TemporaryMemory
{
    MemoryArena *arena;
    uint64 used;
};
inline TemporaryMemory beginTemporaryMemory(MemoryArena *arena)
{
    TemporaryMemory temp = {};
    temp.arena = arena;
    temp.used = arena->used;
    return temp;
}
inline void endTemporaryMemory(TemporaryMemory *temp)
{
    temp->arena->used = temp->used;
}

typedef void *AssetHandle;

#define PLATFORM_LOG_MESSAGE(name) void name(const char *message)
typedef PLATFORM_LOG_MESSAGE(PlatformLogMessage);

#define PLATFORM_QUEUE_ASSET_LOAD(name)                                                       \
    bool name(AssetHandle assetHandle, const char *relativePath)
typedef PLATFORM_QUEUE_ASSET_LOAD(PlatformQueueAssetLoad);

#define PLATFORM_WATCH_ASSET_FILE(name)                                                       \
    void name(AssetHandle assetHandle, const char *relativePath)
typedef PLATFORM_WATCH_ASSET_FILE(PlatformWatchAssetFile);

struct AssetRegistration;
#define PLATFORM_NOTIFY_ASSET_REGISTERED(name) void name(AssetRegistration *assetReg)
typedef PLATFORM_NOTIFY_ASSET_REGISTERED(PlatformNotifyAssetRegistered);

struct EnginePlatformApi
{
    PlatformLogMessage *logMessage;
    PlatformQueueAssetLoad *queueAssetLoad;
    PlatformWatchAssetFile *watchAssetFile;
    PlatformNotifyAssetRegistered *notifyAssetRegistered;
};

#endif