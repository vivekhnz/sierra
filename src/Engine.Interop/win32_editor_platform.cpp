#include "win32_editor_platform.h"
#include "../Engine/terrain_assets.h"

global_variable Win32PlatformMemory *platformMemory;

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

uint64 win32GetFileLastWriteTime(char *path)
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

void win32FreeMemory(void *data)
{
    if (!data)
        return;

    VirtualFree(data, 0, MEM_RELEASE);
}

PLATFORM_LOG_MESSAGE(win32LogMessage)
{
    OutputDebugStringA(message);
}

Win32ReadFileResult win32ReadFile(const char *path)
{
    Win32ReadFileResult result = {};

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
    if (platformMemory->assetLoadQueue.length >= ASSET_LOAD_QUEUE_MAX_SIZE)
    {
        // decline the request - our asset load queue is at capacity
        return false;
    }

    // add asset load request to queue
    uint32 index =
        platformMemory->assetLoadQueue.indices[platformMemory->assetLoadQueue.length++];
    Win32AssetLoadRequest *request = &platformMemory->assetLoadQueue.data[index];
    *request = {};
    request->assetId = assetId;
    request->callback = onAssetLoaded;
    win32GetAbsolutePath(relativePath, request->path);

    // add asset to watch list so we can hot reload it
    bool isAssetAlreadyWatched = false;
    for (int i = 0; i < platformMemory->watchedAssetCount; i++)
    {
        if (platformMemory->watchedAssets[i].assetId == assetId)
        {
            isAssetAlreadyWatched = true;
            break;
        }
    }
    if (!isAssetAlreadyWatched)
    {
        assert(platformMemory->watchedAssetCount < MAX_WATCHED_ASSETS);
        Win32WatchedAsset *watchedAsset =
            &platformMemory->watchedAssets[platformMemory->watchedAssetCount++];
        watchedAsset->assetId = assetId;

        char *src = request->path;
        char *dst = watchedAsset->path;
        while (*src)
        {
            *dst++ = *src++;
        }

        watchedAsset->lastUpdatedTime = win32GetFileLastWriteTime(request->path);
    }

    return true;
}

void win32LoadQueuedAssets(EngineMemory *memory)
{
    // invalidate watched assets that have changed
    for (uint32 i = 0; i < platformMemory->watchedAssetCount; i++)
    {
        Win32WatchedAsset *asset = &platformMemory->watchedAssets[i];
        uint64 lastWriteTime = win32GetFileLastWriteTime(asset->path);
        if (lastWriteTime > asset->lastUpdatedTime)
        {
            asset->lastUpdatedTime = lastWriteTime;
            assetsInvalidateAsset(memory, asset->assetId);
        }
    }

    // action any asset load requests
    for (uint32 i = 0; i < platformMemory->assetLoadQueue.length; i++)
    {
        uint32 index = platformMemory->assetLoadQueue.indices[i];
        Win32AssetLoadRequest *request = &platformMemory->assetLoadQueue.data[index];
        Win32ReadFileResult result = win32ReadFile(request->path);
        if (result.data)
        {
            request->callback(
                &platformMemory->editor.engine, request->assetId, result.data, result.size);
            win32FreeMemory(result.data);

            platformMemory->assetLoadQueue.length--;
            platformMemory->assetLoadQueue.indices[i] =
                platformMemory->assetLoadQueue.indices[platformMemory->assetLoadQueue.length];
            platformMemory->assetLoadQueue.indices[platformMemory->assetLoadQueue.length] =
                index;
            i--;
        }
    }
}

PLATFORM_CAPTURE_MOUSE(win32CaptureMouse)
{
    platformMemory->shouldCaptureMouse = true;
}

Win32PlatformMemory *win32InitializePlatform()
{
#define APP_MEMORY_SIZE (500 * 1024 * 1024)
#define EDITOR_DATA_MEMORY_SIZE (8 * 1024 * 1024)
#define ENGINE_RENDERER_MEMORY_SIZE (1 * 1024 * 1024)
    uint8 *memoryBaseAddress = static_cast<uint8 *>(win32AllocateMemory(APP_MEMORY_SIZE));
    platformMemory = (Win32PlatformMemory *)memoryBaseAddress;
    *platformMemory = {};
    for (uint32 i = 0; i < ASSET_LOAD_QUEUE_MAX_SIZE; i++)
    {
        platformMemory->assetLoadQueue.indices[i] = i;
    }

    platformMemory->editor.platformCaptureMouse = win32CaptureMouse;
    platformMemory->editor.currentState = {};
    platformMemory->editor.newState = {};
    platformMemory->editor.newState.brushRadius = 128.0f;
    platformMemory->editor.newState.brushFalloff = 0.1f;
    platformMemory->editor.newState.brushStrength = 0.12f;
    platformMemory->editor.newState.lightDirection = 0.5f;
    platformMemory->editor.newState.materialCount = 0;
    platformMemory->editor.newState.mode = INTERACTION_MODE_PAINT_BRUSH_STROKE;
    platformMemory->editor.data.baseAddress = memoryBaseAddress + sizeof(Win32PlatformMemory);
    platformMemory->editor.data.size = EDITOR_DATA_MEMORY_SIZE;
    platformMemory->editor.dataStorageUsed = 0;

    EngineMemory *engine = &platformMemory->editor.engine;
    engine->baseAddress =
        (uint8 *)platformMemory->editor.data.baseAddress + platformMemory->editor.data.size;
    engine->size =
        APP_MEMORY_SIZE - (sizeof(Win32PlatformMemory) + platformMemory->editor.data.size);
    engine->platformLogMessage = win32LogMessage;
    engine->platformLoadAsset = win32LoadAsset;

    uint64 engineMemoryOffset = 0;
    engine->renderer.baseAddress = (uint8 *)engine->baseAddress + engineMemoryOffset;
    engine->renderer.size = ENGINE_RENDERER_MEMORY_SIZE;
    engineMemoryOffset += engine->renderer.size;
    engine->assets.baseAddress = (uint8 *)engine->baseAddress + engineMemoryOffset;
    engine->assets.size = engine->size - engineMemoryOffset;
    engineMemoryOffset += engine->assets.size;
    assert(engineMemoryOffset == engine->size);

    return platformMemory;
}