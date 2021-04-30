#include "win32_editor_platform.h"

global_variable Win32PlatformMemory *platformMemory;

void win32ReloadEditorCode(const char *dllPath, const char *dllShadowCopyPath)
{
    Win32EditorCode *editorCode = &platformMemory->editorCode;

    if (editorCode->dllModule)
    {
        FreeLibrary(editorCode->dllModule);
        editorCode->dllModule = 0;
    }

    if (!CopyFileA(dllPath, dllShadowCopyPath, false))
        return;

    editorCode->dllModule = LoadLibraryA(dllShadowCopyPath);
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

    // initialize editor memory
    editorMemory->engineMemory = engineMemory;
    editorMemory->platformCaptureMouse = params->platformCaptureMouse;
    editorMemory->data.baseAddress = editorMemoryBaseAddress + sizeof(EditorMemory);
    editorMemory->data.size = params->editorMemorySize;
    editorMemory->dataStorageUsed = 0;

    return platformMemory;
}