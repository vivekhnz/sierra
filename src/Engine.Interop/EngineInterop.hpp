#pragma once

#include "win32_editor_platform.h"

namespace Terrain { namespace Engine { namespace Interop {
public
    value struct EditorInitPlatformParamsProxy
    {
        System::IntPtr memoryPtr;
        uint64 editorMemorySize;
        uint64 engineMemorySize;

        System::IntPtr platformCaptureMouse;
        System::IntPtr platformLogMessage;
        System::IntPtr platformQueueAssetLoad;
        System::IntPtr platformWatchAssetFile;
    };

public
    ref class EngineInterop
    {
    private:
        static Win32PlatformMemory *memory = nullptr;

    public:
        // platform
        static void InitializeEngine(EditorInitPlatformParamsProxy params);
        static System::IntPtr ReloadEditorCode(
            System::String ^ dllPath, System::String ^ dllShadowCopyPath);

        // engine
        static System::IntPtr GetEngineMemory();

        // editor
        static System::IntPtr GetEditorMemory();
        static void SetEditorEngineApi(System::IntPtr engineApiPtr);
    };
}}}