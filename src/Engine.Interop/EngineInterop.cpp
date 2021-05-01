#include "EngineInterop.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    void EngineInterop::InitializeEngine(EditorInitPlatformParamsProxy params)
    {
        Win32InitPlatformParams initParams = {};
        initParams.memoryBaseAddress = (uint8 *)params.memoryPtr.ToPointer();
        initParams.editorMemorySize = params.editorMemorySize;
        initParams.engineMemorySize = params.engineMemorySize;
        initParams.platformCaptureMouse =
            (PlatformCaptureMouse *)params.platformCaptureMouse.ToPointer();
        initParams.platformLogMessage =
            (PlatformLogMessage *)params.platformLogMessage.ToPointer();
        initParams.platformQueueAssetLoad =
            (PlatformQueueAssetLoad *)params.platformQueueAssetLoad.ToPointer();
        initParams.platformWatchAssetFile =
            (PlatformWatchAssetFile *)params.platformWatchAssetFile.ToPointer();

        memory = win32InitializePlatform(&initParams);
    }

    System::IntPtr EngineInterop::GetEngineMemory()
    {
        return System::IntPtr(memory->engine);
    }

    System::IntPtr EngineInterop::GetEditorMemory()
    {
        return System::IntPtr(memory->editor);
    }

    void EngineInterop::SetEditorEngineApi(System::IntPtr engineApiPtr)
    {
        memory->editor->engineApi = (EngineApi *)engineApiPtr.ToPointer();
    }
}}}