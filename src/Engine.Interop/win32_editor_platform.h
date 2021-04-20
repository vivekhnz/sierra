#ifndef WIN32_EDITOR_PLATFORM_H
#define WIN32_EDITOR_PLATFORM_H

#include <windows.h>

#include "../Engine/engine_platform.h"
#include "../EditorCore/editor.h"

#define ASSET_LOAD_QUEUE_MAX_SIZE 128
#define MAX_WATCHED_ASSETS 256

struct Win32ReadFileResult
{
    void *data;
    uint64 size;
};

struct Win32AssetLoadRequest
{
    uint32 assetId;
    char path[MAX_PATH];
    PlatformAssetLoadCallback *callback;
};

struct Win32AssetLoadQueue
{
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
    EditorUpdateImportedHeightmapTexture *editorUpdateImportedHeightmapTexture;
    EditorRenderHeightmapPreview *editorRenderHeightmapPreview;
};

struct Win32PlatformMemory
{
    Win32EditorCode editorCode;

    Win32AssetLoadQueue assetLoadQueue;
    Win32WatchedAsset watchedAssets[MAX_WATCHED_ASSETS];
    uint32 watchedAssetCount;

    char importedHeightmapTexturePath[MAX_PATH];
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
};
Win32PlatformMemory *win32InitializePlatform(Win32InitPlatformParams *params);

void win32TickApp(float deltaTime, EditorInput *input);
void win32RenderSceneView(EditorViewContext *vctx);
void win32RenderHeightmapPreview(EditorViewContext *vctx);
void win32ShutdownPlatform();

#endif