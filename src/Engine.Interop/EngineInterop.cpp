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

    void EngineInterop::Update(float deltaTime, EditorInputProxy input)
    {
        EditorInput inputInternal = {};
        inputInternal.activeViewState = input.activeViewState.ToPointer();
        inputInternal.scrollOffset = input.scrollOffset;
        inputInternal.normalizedCursorPos =
            glm::vec2(input.normalizedCursorPos.X, input.normalizedCursorPos.Y);
        inputInternal.cursorOffset = glm::vec2(input.cursorOffset.X, input.cursorOffset.Y);
        inputInternal.pressedButtons = input.pressedButtons;
        inputInternal.prevPressedButtons = input.prevPressedButtons;

        if (memory->editorCode.editorUpdate)
        {
            memory->editorCode.editorUpdate(memory->editor, deltaTime, &inputInternal);
        }
    }

    void EngineInterop::RenderSceneView(EditorViewContextProxy % vctx)
    {
        EditorViewContext vctxInternal;
        vctxInternal.viewState = vctx.viewState.ToPointer();
        vctxInternal.x = vctx.x;
        vctxInternal.y = vctx.y;
        vctxInternal.width = vctx.width;
        vctxInternal.height = vctx.height;

        if (memory->editorCode.editorRenderSceneView)
        {
            memory->editorCode.editorRenderSceneView(memory->editor, &vctxInternal);
        }

        vctx.viewState = System::IntPtr(vctxInternal.viewState);
    }

    void EngineInterop::RenderHeightmapPreview(EditorViewContextProxy % vctx)
    {
        EditorViewContext vctxInternal;
        vctxInternal.viewState = vctx.viewState.ToPointer();
        vctxInternal.x = vctx.x;
        vctxInternal.y = vctx.y;
        vctxInternal.width = vctx.width;
        vctxInternal.height = vctx.height;

        if (memory->editorCode.editorRenderHeightmapPreview)
        {
            memory->editorCode.editorRenderHeightmapPreview(memory->editor, &vctxInternal);
        }

        vctx.viewState = System::IntPtr(vctxInternal.viewState);
    }

    TerrainBrushParametersProxy EngineInterop::GetBrushParameters()
    {
        TerrainBrushParametersProxy result = {};

        if (memory->editorCode.editorGetBrushParameters)
        {
            TerrainBrushParameters params =
                memory->editorCode.editorGetBrushParameters(memory->editor);
            result.Radius = params.radius;
            result.Falloff = params.falloff;
            result.Strength = params.strength;
        }

        return result;
    }

    void EngineInterop::AddMaterial(MaterialProps props)
    {
        if (memory->editorCode.editorAddMaterial)
        {
            MaterialProperties matProps = {};
            matProps.albedoTextureAssetId = props.albedoTextureAssetId;
            matProps.normalTextureAssetId = props.normalTextureAssetId;
            matProps.displacementTextureAssetId = props.displacementTextureAssetId;
            matProps.aoTextureAssetId = props.aoTextureAssetId;
            matProps.textureSizeInWorldUnits = props.textureSizeInWorldUnits;
            matProps.slopeStart = props.slopeStart;
            matProps.slopeEnd = props.slopeEnd;
            matProps.altitudeStart = props.altitudeStart;
            matProps.altitudeEnd = props.altitudeEnd;

            memory->editorCode.editorAddMaterial(memory->editor, matProps);
        }
    }

    MaterialProps EngineInterop::GetMaterialProperties(int index)
    {
        MaterialProps result = {};

        if (memory->editorCode.editorGetMaterialProperties)
        {
            MaterialProperties props =
                memory->editorCode.editorGetMaterialProperties(memory->editor, index);
            result.albedoTextureAssetId = props.albedoTextureAssetId;
            result.normalTextureAssetId = props.normalTextureAssetId;
            result.displacementTextureAssetId = props.displacementTextureAssetId;
            result.aoTextureAssetId = props.aoTextureAssetId;
            result.textureSizeInWorldUnits = props.textureSizeInWorldUnits;
            result.slopeStart = props.slopeStart;
            result.slopeEnd = props.slopeEnd;
            result.altitudeStart = props.altitudeStart;
            result.altitudeEnd = props.altitudeEnd;
        }

        return result;
    }
}}}