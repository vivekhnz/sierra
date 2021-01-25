#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

#include <windows.h>

#include "terrain_foundation.h"

struct Win32ReadFileResult
{
    LONGLONG size;
    void *data;
};

EXPORT void *win32AllocateMemory(unsigned int size);
void win32FreeMemory(void *data);
void win32GetAbsolutePath(const char *relativePath, char *absolutePath);
Win32ReadFileResult win32ReadFile(const char *path);

#endif