#ifndef SIERRA_PLATFORM_H
#define SIERRA_PLATFORM_H

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
#undef assert
#define assert(expr)                                                                                              \
    if (!(expr))                                                                                                  \
        *(int *)0 = 0;
#else
#define assert(expr)
#endif

#define arrayCount(array) (sizeof(array) / sizeof(array[0]))
#define offsetOf(type, member) (uint8 *)&((type *)0)->member

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
#define pushArray(arena, type, count) (type *)pushSize(arena, sizeof(type) * (count))

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

#define PLATFORM_LOG_MESSAGE(name) void name(const char *message)
typedef PLATFORM_LOG_MESSAGE(PlatformLogMessage);

#define PLATFORM_GET_FILE_LAST_WRITE_TIME(name) uint64 name(const char *relativePath)
typedef PLATFORM_GET_FILE_LAST_WRITE_TIME(PlatformGetFileLastWriteTime);

#define PLATFORM_GET_FILE_SIZE(name) uint64 name(const char *path)
typedef PLATFORM_GET_FILE_SIZE(PlatformGetFileSize);

#define PLATFORM_READ_ENTIRE_FILE(name) void name(const char *path, void *buffer)
typedef PLATFORM_READ_ENTIRE_FILE(PlatformReadEntireFile);

#define PLATFORM_WRITE_ENTIRE_FILE(name) void name(const char *path, void *buffer, uint64 size)
typedef PLATFORM_WRITE_ENTIRE_FILE(PlatformWriteEntireFile);

struct AssetRegistration;
#define PLATFORM_NOTIFY_ASSET_REGISTERED(name) void name(AssetRegistration *assetReg)
typedef PLATFORM_NOTIFY_ASSET_REGISTERED(PlatformNotifyAssetRegistered);

#define PLATFORM_PUBLISH_TRANSACTION(name) void name(void *commandBufferBaseAddress)
typedef PLATFORM_PUBLISH_TRANSACTION(PlatformPublishTransaction);

#define PLATFORM_START_PERF_COUNTER(name) void name(const char *counterName)
typedef PLATFORM_START_PERF_COUNTER(PlatformStartPerfCounter);

#define PLATFORM_END_PERF_COUNTER(name) void name(const char *counterName)
typedef PLATFORM_END_PERF_COUNTER(PlatformEndPerfCounter);

struct EditorPlatformApi
{
    PlatformLogMessage *logMessage;
    PlatformGetFileLastWriteTime *getFileLastWriteTime;
    PlatformGetFileSize *getFileSize;
    PlatformReadEntireFile *readEntireFile;
    PlatformWriteEntireFile *writeEntireFile;
    PlatformNotifyAssetRegistered *notifyAssetRegistered;
    PlatformPublishTransaction *publishTransaction;
    PlatformStartPerfCounter *startPerfCounter;
    PlatformEndPerfCounter *endPerfCounter;
};

#endif