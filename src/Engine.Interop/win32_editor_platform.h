#ifndef WIN32_EDITOR_PLATFORM_H
#define WIN32_EDITOR_PLATFORM_H

#include <windows.h>

#include "../Engine/terrain_platform.h"
#include "editor.h"
#include "EditorView.h"

#define ASSET_LOAD_QUEUE_MAX_SIZE 128
#define MAX_WATCHED_ASSETS 256
#define MAX_VIEWPORTS 32

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

struct Win32ViewportWindow
{
    HWND hwnd;
    HDC deviceContext;

    Terrain::Engine::Interop::EditorView view;
    EditorViewContext vctx;
};

struct Win32PlatformMemory
{
    HWND mainWindowHwnd;
    HWND dummyWindowHwnd;
    HGLRC glRenderingContext;
    Win32ViewportWindow viewportWindows[MAX_VIEWPORTS];
    uint32 viewportCount;

    bool shouldCaptureMouse;
    bool wasMouseCaptured;
    glm::vec2 prevMousePos_windowSpace;
    glm::vec2 capturedMousePos_windowSpace;
    float nextMouseScrollOffsetY;
    uint64 prevPressedButtons;

    Win32AssetLoadQueue assetLoadQueue;
    Win32WatchedAsset watchedAssets[MAX_WATCHED_ASSETS];
    uint32 watchedAssetCount;

    char importedHeightmapTexturePath[MAX_PATH];
    EditorMemory editor;
};

Win32PlatformMemory *win32InitializePlatform();
Win32ViewportWindow *win32CreateViewportWindow(HWND parentHwnd,
    uint32 x,
    uint32 y,
    uint32 width,
    uint32 height,
    Terrain::Engine::Interop::EditorView view);
Win32ReadFileResult win32ReadFile(const char *path);
void win32FreeMemory(void *data);
void win32TickApp(float deltaTime);
void win32ShutdownPlatform();

#endif