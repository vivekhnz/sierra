#include <windows.h>

#include "win32_platform.h"

#if _DEBUG
#define assert(expr)                                                                          \
    if (!(expr))                                                                              \
        *(int *)0 = 0;
#else
#define assert(expr)
#endif

namespace Terrain { namespace Engine {
    void win32GetAbsolutePath(const char *relativePath, char *absolutePath)
    {
        // get path to current assembly
        CHAR exePath[MAX_PATH];
        GetModuleFileNameA(0, exePath, MAX_PATH);

        // traverse three directories up from the assembly
        int slashPositions[4] = {};
        int slashesFound = 0;
        for (int i = 0; i < MAX_PATH; i++)
        {
            char c = exePath[i];
            if (!c)
            {
                break;
            }
            else if (c == '\\')
            {
                slashesFound++;
                if (slashesFound > 4)
                {
                    slashPositions[0] = slashPositions[1];
                    slashPositions[1] = slashPositions[2];
                    slashPositions[2] = slashPositions[3];
                    slashPositions[3] = i;
                }
                else
                {
                    slashPositions[slashesFound - 1] = i;
                }
            }
        }
        assert(slashesFound >= 4);

        // concatenate root path with relative path
        char *dstCursor = absolutePath;
        for (int i = 0; i < slashPositions[0] + 1; i++)
        {
            *dstCursor++ = exePath[i];
        }
        for (const char *srcCursor = relativePath; *srcCursor; srcCursor++)
        {
            *dstCursor++ = *srcCursor;
        }
        *dstCursor = 0;
    }

    Win32ReadFileResult win32ReadFile(const char *path)
    {
        Win32ReadFileResult result = {};

        HANDLE handle =
            CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        if (handle == INVALID_HANDLE_VALUE)
            return result;

        LARGE_INTEGER size;
        if (GetFileSizeEx(handle, &size))
        {
            result.size = size.QuadPart;
            result.data =
                VirtualAlloc(0, result.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (result.data)
            {
                DWORD bytesRead;
                if (!ReadFile(handle, result.data, result.size, &bytesRead, 0)
                    || result.size != bytesRead)
                {
                    win32FreeMemory(result.data);
                    result.data = 0;
                    result.size = 0;
                }
            }
        }
        CloseHandle(handle);

        return result;
    }

    void win32FreeMemory(void *data)
    {
        if (!data)
            return;

        VirtualFree(data, 0, MEM_RELEASE);
    }
}}