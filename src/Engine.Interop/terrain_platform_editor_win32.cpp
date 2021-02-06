#include "terrain_platform_editor_win32.h"

#include <windows.h>

struct Win32AssetLoadRequest
{
    EngineMemory *memory;
    uint32 assetId;
    char path[MAX_PATH];
    PlatformAssetLoadCallback *callback;
    bool isCompleted;
};

#define ASSET_LOAD_QUEUE_MAX_SIZE 128
global_variable Win32AssetLoadRequest assetLoadQueue[ASSET_LOAD_QUEUE_MAX_SIZE];
global_variable uint32 assetLoadQueueActiveRequestCount;

void getAbsolutePath(const char *relativePath, char *absolutePath)
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

void *win32AllocateMemory(uint64 size)
{
    return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

PLATFORM_FREE_MEMORY(win32FreeMemory)
{
    if (!data)
        return;

    VirtualFree(data, 0, MEM_RELEASE);
}

PLATFORM_READ_FILE(win32ReadFile)
{
    PlatformReadFileResult result = {};

    HANDLE handle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (handle == INVALID_HANDLE_VALUE)
    {
        int err = GetLastError();
        return result;
    }

    LARGE_INTEGER size;
    if (GetFileSizeEx(handle, &size))
    {
        result.size = size.QuadPart;
        result.data = win32AllocateMemory(result.size);
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

PLATFORM_LOAD_ASSET(win32LoadAsset)
{
    assert(assetLoadQueueActiveRequestCount < ASSET_LOAD_QUEUE_MAX_SIZE);

    // todo: reuse space in the asset load queue instead of always appending
    Win32AssetLoadRequest *request = &assetLoadQueue[assetLoadQueueActiveRequestCount++];
    *request = {};
    request->memory = memory;
    request->assetId = assetId;
    request->callback = onAssetLoaded;
    getAbsolutePath(relativePath, request->path);
}

void win32LoadQueuedAssets()
{
    for (uint32 i = 0; i < assetLoadQueueActiveRequestCount; i++)
    {
        Win32AssetLoadRequest *request = &assetLoadQueue[i];
        if (request->isCompleted)
            continue;

        PlatformReadFileResult result = win32ReadFile(request->path);
        if (result.data)
        {
            request->callback(request->memory, request->assetId, &result);
            win32FreeMemory(result.data);
            request->isCompleted = true;
        }
    }
}

uint64 win32GetAssetsLastWriteTime()
{
    char assetsDirectoryPath[MAX_PATH];
    getAbsolutePath("data\\*", assetsDirectoryPath);

    WIN32_FIND_DATAA findResult;
    HANDLE fileHandle = FindFirstFileA(assetsDirectoryPath, &findResult);

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    uint64 mostRecentLastWriteTime = 0;
    do
    {
        if (!(findResult.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            uint64 lastWriteTime = ((uint64)findResult.ftLastWriteTime.dwHighDateTime << 32)
                | findResult.ftLastWriteTime.dwLowDateTime;
            if (lastWriteTime > mostRecentLastWriteTime)
            {
                mostRecentLastWriteTime = lastWriteTime;
            }
        }
    } while (FindNextFileA(fileHandle, &findResult));

    FindClose(fileHandle);

    return mostRecentLastWriteTime;
}