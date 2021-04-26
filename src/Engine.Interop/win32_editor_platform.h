#ifndef WIN32_EDITOR_PLATFORM_H
#define WIN32_EDITOR_PLATFORM_H

#include <windows.h>

#include "../Engine/engine.h"
#include "../EditorCore/editor.h"

struct Win32EngineCode
{
    char dllPath[MAX_PATH];
    char dllShadowCopyPath[MAX_PATH];
    HMODULE dllModule;
    uint64 dllLastWriteTime;

    EngineApi api;
};

struct Win32EditorCode
{
    char dllPath[MAX_PATH];
    char dllShadowCopyPath[MAX_PATH];
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

    char engineCodeDllPath[MAX_PATH];
    char engineCodeDllShadowCopyPath[MAX_PATH];
    char engineCodeBuildLockFilePath[MAX_PATH];

    char editorCodeDllPath[MAX_PATH];
    char editorCodeDllShadowCopyPath[MAX_PATH];
    char editorCodeBuildLockFilePath[MAX_PATH];

    PlatformCaptureMouse *platformCaptureMouse;
    PlatformLogMessage *platformLogMessage;
    PlatformQueueAssetLoad *platformQueueAssetLoad;
    PlatformWatchAssetFile *platformWatchAssetFile;
};
Win32PlatformMemory *win32InitializePlatform(Win32InitPlatformParams *params);
void win32TickPlatform();

#endif