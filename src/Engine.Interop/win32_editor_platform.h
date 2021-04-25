#ifndef WIN32_EDITOR_PLATFORM_H
#define WIN32_EDITOR_PLATFORM_H

#include <windows.h>

#include "../Engine/engine.h"
#include "../EditorCore/editor.h"

#define MAX_WATCHED_ASSETS 256

struct Win32WatchedAsset
{
    uint32 assetId;
    char path[MAX_PATH];
    uint64 lastUpdatedTime;
};

struct Win32EditorCode
{
    char dllPath[MAX_PATH];
    char dllShadowCopyPath[MAX_PATH];
    char buildLockFilePath[MAX_PATH];
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
    EnginePlatformApi engineApi;
    Win32EditorCode editorCode;

    Win32WatchedAsset watchedAssets[MAX_WATCHED_ASSETS];
    uint32 watchedAssetCount;

    EditorMemory *editor;
};

struct Win32InitPlatformParams
{
    uint8 *memoryBaseAddress;
    uint64 editorMemorySize;
    uint64 engineMemorySize;

    char editorCodeDllPath[MAX_PATH];
    char editorCodeDllShadowCopyPath[MAX_PATH];
    char editorCodeBuildLockFilePath[MAX_PATH];

    PlatformCaptureMouse *platformCaptureMouse;
    PlatformLogMessage *platformLogMessage;
    PlatformQueueAssetLoad *platformQueueAssetLoad;
};
Win32PlatformMemory *win32InitializePlatform(Win32InitPlatformParams *params);
void win32TickPlatform();

#endif