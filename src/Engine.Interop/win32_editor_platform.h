#ifndef WIN32_EDITOR_PLATFORM_H
#define WIN32_EDITOR_PLATFORM_H

#include <windows.h>

#include "../Engine/engine.h"
#include "../EditorCore/editor.h"

struct Win32EngineCode
{
    HMODULE dllModule;
    uint64 dllLastWriteTime;

    EngineApi api;
};

struct Win32EditorCode
{
    HMODULE dllModule;
    uint64 dllLastWriteTime;

    EditorUpdate *editorUpdate;
    EditorShutdown *editorShutdown;
    EditorRenderSceneView *editorRenderSceneView;
    EditorRenderHeightmapPreview *editorRenderHeightmapPreview;
    EditorGetImportedHeightmapAssetId *editorGetImportedHeightmapAssetId;
};

struct Win32PlatformMemory
{
    char buildLockFilePath[MAX_PATH];
    Win32EngineCode engineCode;
    Win32EditorCode editorCode;

    EngineMemory *engine;
    EditorMemory *editor;
};

struct Win32InitPlatformParams
{
    uint8 *memoryBaseAddress;
    uint64 editorMemorySize;
    uint64 engineMemorySize;

    char buildLockFilePath[MAX_PATH];
    char engineCodeDllPath[MAX_PATH];
    char engineCodeDllShadowCopyPath[MAX_PATH];

    PlatformCaptureMouse *platformCaptureMouse;
    PlatformLogMessage *platformLogMessage;
    PlatformQueueAssetLoad *platformQueueAssetLoad;
    PlatformWatchAssetFile *platformWatchAssetFile;
};
Win32PlatformMemory *win32InitializePlatform(Win32InitPlatformParams *params);
void win32TickPlatform(const char *engineCodeDllPath,
    const char *engineCodeDllShadowCopyPath,
    const char *editorCodeDllPath,
    const char *editorCodeDllShadowCopyPath);

#endif