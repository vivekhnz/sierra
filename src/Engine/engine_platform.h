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

struct MemoryBlock
{
    void *baseAddress;
    uint64 size;
};

struct EngineMemory;

#define PLATFORM_GET_GL_PROC_ADDRESS(name) void *name(const char *procName)
typedef PLATFORM_GET_GL_PROC_ADDRESS(PlatformGetGlProcAddress);

#define PLATFORM_LOG_MESSAGE(name) void name(const char *message)
typedef PLATFORM_LOG_MESSAGE(PlatformLogMessage);

#define PLATFORM_LOAD_ASSET(name) bool name(uint32 assetId, const char *relativePath)
typedef PLATFORM_LOAD_ASSET(PlatformLoadAsset);

struct EngineMemory
{
    PlatformGetGlProcAddress *platformGetGlProcAddress;
    PlatformLogMessage *platformLogMessage;
    PlatformLoadAsset *platformLoadAsset;

    MemoryBlock renderer;
    MemoryBlock assets;
};

#endif