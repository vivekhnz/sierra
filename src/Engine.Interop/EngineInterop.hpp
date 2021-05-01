#pragma once

#include "../EditorCore/editor.h"

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
        static EngineMemory *engineMemory = nullptr;
        static EditorMemory *editorMemory = nullptr;

    public:
        // platform
        static void InitializeEngine(EditorInitPlatformParamsProxy params);

        // engine
        static System::IntPtr GetEngineMemory();

        // editor
        static System::IntPtr GetEditorMemory();
        static void SetEditorEngineApi(System::IntPtr engineApiPtr);
    };
}}}