#include "EngineInterop.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    void EngineInterop::InitializeEngine(EditorInitPlatformParamsProxy params)
    {
        uint8 *editorMemoryBaseAddress = (uint8 *)params.editorMemoryPtr.ToPointer();
        uint8 *engineMemoryBaseAddress = (uint8 *)params.engineMemoryPtr.ToPointer();

        editorMemory = (EditorMemory *)editorMemoryBaseAddress;

        // initialize editor memory
        editorMemory->engineMemory = (EngineMemory *)engineMemoryBaseAddress;
        editorMemory->platformCaptureMouse =
            (PlatformCaptureMouse *)params.platformCaptureMouse.ToPointer();
        editorMemory->data.baseAddress = editorMemoryBaseAddress + sizeof(EditorMemory);
        editorMemory->data.size = params.editorMemorySize;
        editorMemory->dataStorageUsed = 0;
    }

    void EngineInterop::SetEditorEngineApi(System::IntPtr engineApiPtr)
    {
        editorMemory->engineApi = (EngineApi *)engineApiPtr.ToPointer();
    }
}}}