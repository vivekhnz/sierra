#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

#include <windows.h>

#include "terrain_platform.h"

EXPORT void *win32AllocateMemory(uint64 size);
PLATFORM_FREE_MEMORY(win32FreeMemory);
PLATFORM_READ_FILE(win32ReadFile);
PLATFORM_LOAD_ASSET(win32LoadAsset);

#endif