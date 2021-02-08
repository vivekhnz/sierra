#include <windows.h>

#include "terrain_platform_win32.h"
#include "../Engine/terrain_assets.h"

#define ASSET_LOAD_QUEUE_MAX_SIZE 128
#define MAX_WATCHED_ASSETS 256

struct Win32AssetLoadRequest
{
    EngineMemory *memory;
    uint32 assetId;
    char path[MAX_PATH];
    PlatformAssetLoadCallback *callback;
};

struct Win32AssetLoadQueue
{
    bool isInitialized;
    uint32 length;
    Win32AssetLoadRequest data[ASSET_LOAD_QUEUE_MAX_SIZE];
    uint32 indices[ASSET_LOAD_QUEUE_MAX_SIZE];
};

struct Win32WatchedAsset
{
    uint32 assetId;
    char path[MAX_PATH];
    uint64 lastUpdatedTime;
};

global_variable Win32AssetLoadQueue assetLoadQueue;
global_variable Win32WatchedAsset watchedAssets[MAX_WATCHED_ASSETS];
global_variable uint32 watchedAssetCount;

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

uint64 getFileLastWriteTime(char *path)
{
    WIN32_FILE_ATTRIBUTE_DATA attributes;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attributes))
    {
        return 0;
    }

    uint64 lastWriteTime = ((uint64)attributes.ftLastWriteTime.dwHighDateTime << 32)
        | attributes.ftLastWriteTime.dwLowDateTime;
    return lastWriteTime;
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
    if (!assetLoadQueue.isInitialized)
    {
        for (uint32 i = 0; i < ASSET_LOAD_QUEUE_MAX_SIZE; i++)
        {
            assetLoadQueue.indices[i] = i;
        }
        assetLoadQueue.isInitialized = true;
    }

    if (assetLoadQueue.length >= ASSET_LOAD_QUEUE_MAX_SIZE)
    {
        // decline the request - our asset load queue is at capacity
        return false;
    }

    // add asset load request to queue
    uint32 index = assetLoadQueue.indices[assetLoadQueue.length++];
    Win32AssetLoadRequest *request = &assetLoadQueue.data[index];
    *request = {};
    request->memory = memory;
    request->assetId = assetId;
    request->callback = onAssetLoaded;
    getAbsolutePath(relativePath, request->path);

    // add asset to watch list so we can hot reload it
    bool isAssetAlreadyWatched = false;
    for (int i = 0; i < watchedAssetCount; i++)
    {
        if (watchedAssets[i].assetId == assetId)
        {
            isAssetAlreadyWatched = true;
            break;
        }
    }
    if (!isAssetAlreadyWatched)
    {
        assert(watchedAssetCount < MAX_WATCHED_ASSETS);
        Win32WatchedAsset *watchedAsset = &watchedAssets[watchedAssetCount++];
        watchedAsset->assetId = assetId;

        char *src = request->path;
        char *dst = watchedAsset->path;
        while (*src)
        {
            *dst++ = *src++;
        }

        watchedAsset->lastUpdatedTime = getFileLastWriteTime(request->path);
    }

    return true;
}

void win32LoadQueuedAssets(EngineMemory *memory)
{
    // invalidate watched assets that have changed
    for (uint32 i = 0; i < watchedAssetCount; i++)
    {
        Win32WatchedAsset *asset = &watchedAssets[i];
        uint64 lastWriteTime = getFileLastWriteTime(asset->path);
        if (lastWriteTime > asset->lastUpdatedTime)
        {
            asset->lastUpdatedTime = lastWriteTime;
            assetsInvalidateShader(memory, asset->assetId);
        }
    }

    // action any asset load requests
    for (uint32 i = 0; i < assetLoadQueue.length; i++)
    {
        uint32 index = assetLoadQueue.indices[i];
        Win32AssetLoadRequest *request = &assetLoadQueue.data[index];
        PlatformReadFileResult result = win32ReadFile(request->path);
        if (result.data)
        {
            request->callback(request->memory, request->assetId, &result);
            win32FreeMemory(result.data);

            assetLoadQueue.length--;
            assetLoadQueue.indices[i] = assetLoadQueue.indices[assetLoadQueue.length];
            assetLoadQueue.indices[assetLoadQueue.length] = index;
            i--;
        }
    }
}