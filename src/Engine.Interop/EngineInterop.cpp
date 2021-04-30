#include "EngineInterop.hpp"

#include <msclr\marshal_cppstd.h>

namespace Terrain { namespace Engine { namespace Interop {
    void GetCStringFromManagedString(System::String ^ src, char *dst)
    {
        std::string stdStr = msclr::interop::marshal_as<std::string>(src);
        const char *srcCursor = stdStr.c_str();
        char *dstCursor = dst;
        while (*srcCursor)
        {
            *dstCursor++ = *srcCursor++;
        }
        *dstCursor = 0;
    }

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

    System::IntPtr EngineInterop::ReloadEditorCode(
        System::String ^ dllPath, System::String ^ dllShadowCopyPath)
    {
        char dllPathCStr[MAX_PATH];
        char dllShadowCopyPathCStr[MAX_PATH];
        GetCStringFromManagedString(dllPath, dllPathCStr);
        GetCStringFromManagedString(dllShadowCopyPath, dllShadowCopyPathCStr);
        win32ReloadEditorCode(dllPathCStr, dllShadowCopyPathCStr);

        return System::IntPtr(memory->editorCode.dllModule);
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