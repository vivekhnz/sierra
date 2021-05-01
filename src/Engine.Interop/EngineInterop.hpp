#pragma once

#include "../EditorCore/editor.h"

namespace Terrain { namespace Engine { namespace Interop {
public
    value struct EditorInitPlatformParamsProxy
    {
        System::IntPtr editorMemoryPtr;
        uint64 editorMemorySize;

        System::IntPtr engineMemoryPtr;
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
        static EditorMemory *editorMemory = nullptr;

    public:
        // platform
        static void InitializeEngine(EditorInitPlatformParamsProxy params);

        // editor
        static void SetEditorEngineApi(System::IntPtr engineApiPtr);
    };
}}}