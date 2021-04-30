#ifndef WIN32_EDITOR_PLATFORM_H
#define WIN32_EDITOR_PLATFORM_H

#include <windows.h>

#include "../Engine/engine.h"
#include "../EditorCore/editor.h"

struct Win32EditorCode
{
    HMODULE dllModule;
};

struct Win32PlatformMemory
{
    Win32EditorCode editorCode;

    EngineMemory *engine;
    EditorMemory *editor;
};

struct Win32InitPlatformParams
{
    uint8 *memoryBaseAddress;
    uint64 editorMemorySize;
    uint64 engineMemorySize;

    PlatformCaptureMouse *platformCaptureMouse;
    PlatformLogMessage *platformLogMessage;
    PlatformQueueAssetLoad *platformQueueAssetLoad;
    PlatformWatchAssetFile *platformWatchAssetFile;
};
Win32PlatformMemory *win32InitializePlatform(Win32InitPlatformParams *params);
void win32ReloadEditorCode(const char *dllPath, const char *dllShadowCopyPath);

#endif