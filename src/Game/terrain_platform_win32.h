#ifndef TERRAIN_PLATFORM_WIN32_H
#define TERRAIN_PLATFORM_WIN32_H

#include <windows.h>

#include "../Engine/terrain_platform.h"

EXPORT void *win32AllocateMemory(uint64 size);
EXPORT PLATFORM_FREE_MEMORY(win32FreeMemory);
EXPORT PLATFORM_READ_FILE(win32ReadFile);
EXPORT PLATFORM_LOAD_ASSET(win32LoadAsset);

#endif