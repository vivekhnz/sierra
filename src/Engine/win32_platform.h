#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

#include <windows.h>

#include "terrain_foundation.h"

struct PlatformReadFileResult
{
    uint64 size;
    void *data;
};

typedef void AssetLoadCallback(
    MemoryBlock *memory, uint32 assetId, PlatformReadFileResult *result);

EXPORT void *win32AllocateMemory(uint64 size);
void win32FreeMemory(void *data);
PlatformReadFileResult win32ReadFile(const char *path);
PlatformReadFileResult win32LoadAsset(MemoryBlock *memory,
    uint32 assetId,
    const char *relativePath,
    AssetLoadCallback onAssetLoaded);

#endif