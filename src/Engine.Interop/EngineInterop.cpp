#include "EngineInterop.hpp"

namespace Terrain { namespace Engine { namespace Interop {
    void EngineInterop::InitializeEngine(EditorInitPlatformParamsProxy params)
    {
        uint8 *editorMemoryBaseAddress = (uint8 *)params.memoryPtr.ToPointer();
        uint8 *engineMemoryBaseAddress = editorMemoryBaseAddress + params.editorMemorySize;

        editorMemory = (EditorMemory *)editorMemoryBaseAddress;
        engineMemory = (EngineMemory *)engineMemoryBaseAddress;

        // initialize engine memory
        engineMemory->platformLogMessage =
            (PlatformLogMessage *)params.platformLogMessage.ToPointer();
        engineMemory->platformQueueAssetLoad =
            (PlatformQueueAssetLoad *)params.platformQueueAssetLoad.ToPointer();
        engineMemory->platformWatchAssetFile =
            (PlatformWatchAssetFile *)params.platformWatchAssetFile.ToPointer();

#define ENGINE_RENDERER_MEMORY_SIZE (1 * 1024 * 1024)
        uint64 engineMemoryOffset = sizeof(EngineMemory);
        engineMemory->renderer.baseAddress = engineMemoryBaseAddress + engineMemoryOffset;
        engineMemory->renderer.size = ENGINE_RENDERER_MEMORY_SIZE;
        engineMemoryOffset += engineMemory->renderer.size;
        engineMemory->assets.baseAddress = engineMemoryBaseAddress + engineMemoryOffset;
        engineMemory->assets.size = params.engineMemorySize - engineMemoryOffset;
        engineMemoryOffset += engineMemory->assets.size;
        assert(engineMemoryOffset == params.engineMemorySize);

        // initialize editor memory
        editorMemory->engineMemory = engineMemory;
        editorMemory->platformCaptureMouse =
            (PlatformCaptureMouse *)params.platformCaptureMouse.ToPointer();
        editorMemory->data.baseAddress = editorMemoryBaseAddress + sizeof(EditorMemory);
        editorMemory->data.size = params.editorMemorySize;
        editorMemory->dataStorageUsed = 0;
    }

    System::IntPtr EngineInterop::GetEngineMemory()
    {
        return System::IntPtr(engineMemory);
    }

    System::IntPtr EngineInterop::GetEditorMemory()
    {
        return System::IntPtr(editorMemory);
    }

    void EngineInterop::SetEditorEngineApi(System::IntPtr engineApiPtr)
    {
        editorMemory->engineApi = (EngineApi *)engineApiPtr.ToPointer();
    }
}}}