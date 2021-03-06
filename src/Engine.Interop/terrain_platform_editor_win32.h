#pragma once

#include "../Engine/terrain_platform.h"

struct Win32ReadFileResult
{
    void *data;
    uint64 size;
};

EXPORT void *win32AllocateMemory(uint64 size);
EXPORT PLATFORM_LOG_MESSAGE(win32LogMessage);
EXPORT PLATFORM_LOAD_ASSET(win32LoadAsset);

Win32ReadFileResult win32ReadFile(const char *path);
void win32FreeMemory(void *data);
void win32LoadQueuedAssets(EngineMemory *memory);