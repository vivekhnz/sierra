#include "win32_editor_platform.h"

using namespace System::Windows;
using namespace System::Windows::Input;

global_variable Win32PlatformMemory *platformMemory;

uint64 win32GetFileLastWriteTime(const char *path)
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

void win32ReloadEngineCode(const char *dllPath, const char *dllShadowCopyPath)
{
    Win32EngineCode *engineCode = &platformMemory->engineCode;

    if (engineCode->dllModule)
    {
        FreeLibrary(engineCode->dllModule);
        engineCode->dllModule = 0;
    }

    while (!CopyFileA(dllPath, dllShadowCopyPath, false))
    {
        Sleep(100);
    }

    engineCode->dllModule = LoadLibraryA(dllShadowCopyPath);
    if (engineCode->dllModule)
    {
        EngineGetApi *engineGetApi =
            (EngineGetApi *)GetProcAddress(engineCode->dllModule, "engineGetApi");
        engineGetApi(&engineCode->api);

        engineCode->api.rendererInitialize(platformMemory->engine, 0);
    }
}

void win32ReloadEditorCode(const char *dllPath, const char *dllShadowCopyPath)
{
    Win32EditorCode *editorCode = &platformMemory->editorCode;

    if (editorCode->dllModule)
    {
        FreeLibrary(editorCode->dllModule);
        editorCode->dllModule = 0;

        editorCode->editorUpdate = 0;
        editorCode->editorShutdown = 0;
        editorCode->editorRenderSceneView = 0;
        editorCode->editorRenderHeightmapPreview = 0;
    }

    if (!CopyFileA(dllPath, dllShadowCopyPath, false))
        return;

    editorCode->dllModule = LoadLibraryA(dllShadowCopyPath);
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
    platformMemory->engine = engineMemory;
    platformMemory->editor = editorMemory;
    win32CopyString(params->buildLockFilePath, platformMemory->buildLockFilePath);

    // initialize engine memory
    engineMemory->platformLogMessage = params->platformLogMessage;
    engineMemory->platformQueueAssetLoad = params->platformQueueAssetLoad;
    engineMemory->platformWatchAssetFile = params->platformWatchAssetFile;

#define ENGINE_RENDERER_MEMORY_SIZE (1 * 1024 * 1024)
    uint64 engineMemoryOffset = sizeof(EngineMemory);
    engineMemory->renderer.baseAddress = engineMemoryBaseAddress + engineMemoryOffset;
    engineMemory->renderer.size = ENGINE_RENDERER_MEMORY_SIZE;
    engineMemoryOffset += engineMemory->renderer.size;
    engineMemory->assets.baseAddress = engineMemoryBaseAddress + engineMemoryOffset;
    engineMemory->assets.size = actualEngineMemorySize - engineMemoryOffset;
    engineMemoryOffset += engineMemory->assets.size;
    assert(engineMemoryOffset == actualEngineMemorySize);

    // load engine code
    win32ReloadEngineCode(params->engineCodeDllPath, params->engineCodeDllShadowCopyPath);
    platformMemory->engineCode.dllLastWriteTime =
        win32GetFileLastWriteTime(params->engineCodeDllPath);

    // initialize editor memory
    editorMemory->engineMemory = engineMemory;
    editorMemory->engineApi = &platformMemory->engineCode.api;
    editorMemory->platformCaptureMouse = params->platformCaptureMouse;
    editorMemory->data.baseAddress = editorMemoryBaseAddress + sizeof(EditorMemory);
    editorMemory->data.size = params->editorMemorySize;
    editorMemory->dataStorageUsed = 0;

    return platformMemory;
}

void win32TickPlatform(const char *engineCodeDllPath,
    const char *engineCodeDllShadowCopyPath,
    const char *editorCodeDllPath,
    const char *editorCodeDllShadowCopyPath)
{
    if (!win32GetFileLastWriteTime(platformMemory->buildLockFilePath))
    {
        uint64 engineCodeDllLastWriteTime = win32GetFileLastWriteTime(engineCodeDllPath);
        if (engineCodeDllLastWriteTime
            && engineCodeDllLastWriteTime > platformMemory->engineCode.dllLastWriteTime)
        {
            win32ReloadEngineCode(engineCodeDllPath, engineCodeDllShadowCopyPath);
            platformMemory->engineCode.dllLastWriteTime = engineCodeDllLastWriteTime;
        }

        uint64 editorCodeDllLastWriteTime = win32GetFileLastWriteTime(editorCodeDllPath);
        if (editorCodeDllLastWriteTime
            && editorCodeDllLastWriteTime > platformMemory->editorCode.dllLastWriteTime)
        {
            win32ReloadEditorCode(editorCodeDllPath, editorCodeDllShadowCopyPath);
            platformMemory->editorCode.dllLastWriteTime = editorCodeDllLastWriteTime;
        }
    }
}