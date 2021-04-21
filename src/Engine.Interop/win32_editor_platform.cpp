#include "win32_editor_platform.h"

#include <windowsx.h>
#include "../Engine/engine_assets.h"

using namespace System::Windows;
using namespace System::Windows::Input;

global_variable Win32PlatformMemory *platformMemory;

void win32GetAssetAbsolutePath(const char *relativePath, char *absolutePath)
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
    win32GetAssetAbsolutePath(relativePath, request->path);

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

void win32LoadQueuedAssets()
{
    // invalidate watched assets that have changed
    for (uint32 i = 0; i < platformMemory->watchedAssetCount; i++)
    {
        Win32WatchedAsset *asset = &platformMemory->watchedAssets[i];
        uint64 lastWriteTime = win32GetFileLastWriteTime(asset->path);
        if (lastWriteTime > asset->lastUpdatedTime)
        {
            asset->lastUpdatedTime = lastWriteTime;
            assetsInvalidateAsset(platformMemory->editor->engine, asset->assetId);
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
            assetsOnAssetLoaded(
                platformMemory->editor->engine, request->assetId, result.data, result.size);
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

void win32CopyString(char *src, char *dst)
{
    char *srcCursor = src;
    char *dstCursor = dst;
    while (*srcCursor)
    {
        *dstCursor++ = *srcCursor++;
    }
}

Win32PlatformMemory *win32InitializePlatform(Win32InitPlatformParams *params)
{
    uint8 *platformMemoryBaseAddress = params->memoryBaseAddress;
    uint8 *editorMemoryBaseAddress = platformMemoryBaseAddress + sizeof(Win32PlatformMemory);
    uint8 *engineMemoryBaseAddress = editorMemoryBaseAddress + params->editorMemorySize;
    uint8 actualEngineMemorySize = params->engineMemorySize - sizeof(Win32PlatformMemory);

    platformMemory = (Win32PlatformMemory *)platformMemoryBaseAddress;
    EditorMemory *editorMemory = (EditorMemory *)editorMemoryBaseAddress;
    EngineMemory *engineMemory = (EngineMemory *)engineMemoryBaseAddress;

    // initialize platform memory
    *platformMemory = {};
    platformMemory->editor = editorMemory;
    for (uint32 i = 0; i < ASSET_LOAD_QUEUE_MAX_SIZE; i++)
    {
        platformMemory->assetLoadQueue.indices[i] = i;
    }
    win32CopyString(params->editorCodeDllPath, platformMemory->editorCode.dllPath);
    win32CopyString(
        params->editorCodeDllShadowCopyPath, platformMemory->editorCode.dllShadowCopyPath);
    win32CopyString(
        params->editorCodeBuildLockFilePath, platformMemory->editorCode.buildLockFilePath);

    // initialize editor memory
    editorMemory->platformCaptureMouse = params->platformCaptureMouse;
    editorMemory->state.uiState = {};
    editorMemory->state.uiState.brushRadius = 128.0f;
    editorMemory->state.uiState.brushFalloff = 0.75f;
    editorMemory->state.uiState.brushStrength = 0.12f;
    editorMemory->state.uiState.lightDirection = 0.5f;
    editorMemory->state.uiState.materialCount = 0;
    editorMemory->state.uiState.rockPosition = glm::vec3(0);
    editorMemory->state.uiState.rockRotation = glm::vec3(0);
    editorMemory->state.uiState.rockScale = glm::vec3(1);
    editorMemory->data.baseAddress = editorMemoryBaseAddress + sizeof(EditorMemory);
    editorMemory->data.size = params->editorMemorySize;
    editorMemory->dataStorageUsed = 0;
    editorMemory->engine = engineMemory;

    // initialize engine memory
    engineMemory->platformLogMessage = win32LogMessage;
    engineMemory->platformLoadAsset = win32LoadAsset;

#define ENGINE_RENDERER_MEMORY_SIZE (1 * 1024 * 1024)
    uint64 engineMemoryOffset = sizeof(EngineMemory);
    engineMemory->renderer.baseAddress = engineMemoryBaseAddress + engineMemoryOffset;
    engineMemory->renderer.size = ENGINE_RENDERER_MEMORY_SIZE;
    engineMemoryOffset += engineMemory->renderer.size;
    engineMemory->assets.baseAddress = engineMemoryBaseAddress + engineMemoryOffset;
    engineMemory->assets.size = actualEngineMemorySize - engineMemoryOffset;
    engineMemoryOffset += engineMemory->assets.size;
    assert(engineMemoryOffset == actualEngineMemorySize);

    return platformMemory;
}

void win32LoadEditorCode(Win32EditorCode *editorCode)
{
    if (!CopyFileA(editorCode->dllPath, editorCode->dllShadowCopyPath, false))
        return;

    editorCode->dllModule = LoadLibraryA(editorCode->dllShadowCopyPath);
    if (editorCode->dllModule)
    {
        editorCode->editorUpdate =
            (EditorUpdate *)GetProcAddress(editorCode->dllModule, "editorUpdate");
        editorCode->editorShutdown =
            (EditorShutdown *)GetProcAddress(editorCode->dllModule, "editorShutdown");
        editorCode->editorRenderSceneView = (EditorRenderSceneView *)GetProcAddress(
            editorCode->dllModule, "editorRenderSceneView");
        editorCode->editorUpdateImportedHeightmapTexture =
            (EditorUpdateImportedHeightmapTexture *)GetProcAddress(
                editorCode->dllModule, "editorUpdateImportedHeightmapTexture");
        editorCode->editorRenderHeightmapPreview =
            (EditorRenderHeightmapPreview *)GetProcAddress(
                editorCode->dllModule, "editorRenderHeightmapPreview");
    }
}

void win32UnloadEditorCode(Win32EditorCode *editorCode)
{
    if (editorCode->dllModule)
    {
        FreeLibrary(editorCode->dllModule);
        editorCode->dllModule = 0;

        editorCode->editorUpdate = 0;
        editorCode->editorShutdown = 0;
        editorCode->editorRenderSceneView = 0;
        editorCode->editorUpdateImportedHeightmapTexture = 0;
        editorCode->editorRenderHeightmapPreview = 0;
    }
}

void win32TickApp(float deltaTime, EditorInput *input)
{
    uint64 editorCodeDllLastWriteTime =
        win32GetFileLastWriteTime(platformMemory->editorCode.dllPath);
    if (editorCodeDllLastWriteTime
        && editorCodeDllLastWriteTime > platformMemory->editorCode.dllLastWriteTime
        && !win32GetFileLastWriteTime(platformMemory->editorCode.buildLockFilePath))
    {
        win32UnloadEditorCode(&platformMemory->editorCode);
        win32LoadEditorCode(&platformMemory->editorCode);
        platformMemory->editorCode.dllLastWriteTime = editorCodeDllLastWriteTime;
    }

    win32LoadQueuedAssets();

    if (platformMemory->importedHeightmapTexturePath[0]
        && platformMemory->editorCode.editorUpdateImportedHeightmapTexture)
    {
        Win32ReadFileResult result =
            win32ReadFile(platformMemory->importedHeightmapTexturePath);
        assert(result.data);

        TextureAsset asset;
        assetsLoadTexture(
            platformMemory->editor->engine, result.data, result.size, true, &asset);
        platformMemory->editorCode.editorUpdateImportedHeightmapTexture(
            platformMemory->editor, &asset);

        win32FreeMemory(result.data);
        *platformMemory->importedHeightmapTexturePath = 0;
    }

    if (platformMemory->editorCode.editorUpdate)
    {
        platformMemory->editorCode.editorUpdate(platformMemory->editor, deltaTime, input);
    }
}

void win32RenderSceneView(EditorViewContext *vctx)
{
    if (platformMemory->editorCode.editorRenderSceneView)
    {
        platformMemory->editorCode.editorRenderSceneView(platformMemory->editor, vctx);
    }
}

void win32RenderHeightmapPreview(EditorViewContext *vctx)
{
    if (platformMemory->editorCode.editorRenderHeightmapPreview)
    {
        platformMemory->editorCode.editorRenderHeightmapPreview(platformMemory->editor, vctx);
    }
}

void win32ShutdownPlatform()
{
    if (platformMemory->editorCode.editorShutdown)
    {
        platformMemory->editorCode.editorShutdown(platformMemory->editor);
    }
}