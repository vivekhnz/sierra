#include "win32_editor_platform.h"

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
    *dstCursor++ = 'd';
    *dstCursor++ = 'a';
    *dstCursor++ = 't';
    *dstCursor++ = 'a';
    *dstCursor++ = '\\';
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

void win32CopyString(const char *src, char *dst)
{
    const char *srcCursor = src;
    char *dstCursor = dst;
    while (*srcCursor)
    {
        *dstCursor++ = *srcCursor++;
    }
}

PLATFORM_WATCH_ASSET_FILE(win32WatchAssetFile)
{
    if (platformMemory->watchedAssetCount >= MAX_WATCHED_ASSETS)
    {
        return;
    }
    Win32WatchedAsset *watchedAsset =
        &platformMemory->watchedAssets[platformMemory->watchedAssetCount++];
    watchedAsset->assetId = assetId;
    win32GetAssetAbsolutePath(relativePath, watchedAsset->path);
    watchedAsset->lastUpdatedTime = win32GetFileLastWriteTime(watchedAsset->path);
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
    engineGetPlatformApi(&platformMemory->engineApi);
    platformMemory->editor = editorMemory;
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
    engineGetClientApi(&editorMemory->engine);
    editorMemory->engineMemory = engineMemory;

    // initialize engine memory
    engineMemory->platformLogMessage = params->platformLogMessage;
    engineMemory->platformQueueAssetLoad = params->platformQueueAssetLoad;
    engineMemory->platformWatchAssetFile = win32WatchAssetFile;

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
        editorCode->editorRenderHeightmapPreview =
            (EditorRenderHeightmapPreview *)GetProcAddress(
                editorCode->dllModule, "editorRenderHeightmapPreview");
        editorCode->editorGetImportedHeightmapAssetId =
            (EditorGetImportedHeightmapAssetId *)GetProcAddress(
                editorCode->dllModule, "editorGetImportedHeightmapAssetId");
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
        editorCode->editorRenderHeightmapPreview = 0;
    }
}

void win32TickPlatform()
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

    // invalidate watched assets that have changed
    for (uint32 i = 0; i < platformMemory->watchedAssetCount; i++)
    {
        Win32WatchedAsset *asset = &platformMemory->watchedAssets[i];
        uint64 lastWriteTime = win32GetFileLastWriteTime(asset->path);
        if (lastWriteTime > asset->lastUpdatedTime)
        {
            asset->lastUpdatedTime = lastWriteTime;
            platformMemory->engineApi.assetsInvalidateAsset(
                platformMemory->editor->engineMemory, asset->assetId);
        }
    }
}