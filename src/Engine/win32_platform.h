#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

namespace Terrain { namespace Engine {

    struct Win32ReadFileResult
    {
        LONGLONG size;
        void *data;
    };

    void win32GetAbsolutePath(const char *relativePath, char *absolutePath);
    Win32ReadFileResult win32ReadFile(const char *path);
    void win32FreeMemory(void *data);
}}

#endif