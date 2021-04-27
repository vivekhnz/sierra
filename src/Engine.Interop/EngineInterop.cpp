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

    void EngineInterop::ReloadEngineCode(
        System::String ^ dllPath, System::String ^ dllShadowCopyPath)
    {
        char dllPathCStr[MAX_PATH];
        char dllShadowCopyPathCStr[MAX_PATH];
        GetCStringFromManagedString(dllPath, dllPathCStr);
        GetCStringFromManagedString(dllShadowCopyPath, dllShadowCopyPathCStr);
        win32ReloadEngineCode(dllPathCStr, dllShadowCopyPathCStr);
    }

    void EngineInterop::ReloadEditorCode(
        System::String ^ dllPath, System::String ^ dllShadowCopyPath)
    {
        char dllPathCStr[MAX_PATH];
        char dllShadowCopyPathCStr[MAX_PATH];
        GetCStringFromManagedString(dllPath, dllPathCStr);
        GetCStringFromManagedString(dllShadowCopyPath, dllShadowCopyPathCStr);
        win32ReloadEditorCode(dllPathCStr, dllShadowCopyPathCStr);
    }

    uint32 EngineInterop::GetRegisteredAssetCount()
    {
        return memory->engineCode.api->assetsGetRegisteredAssetCount(
            memory->editor->engineMemory);
    }

    array<AssetRegistrationProxy> ^ EngineInterop::GetRegisteredAssets()
    {
        uint32 assetCount = GetRegisteredAssetCount();
        AssetRegistration *assetRegs =
            memory->engineCode.api->assetsGetRegisteredAssets(memory->editor->engineMemory);

        array<AssetRegistrationProxy> ^ result =
            gcnew array<AssetRegistrationProxy>(assetCount);
        for (uint32 i = 0; i < assetCount; i++)
        {
            AssetRegistration *reg = &assetRegs[i];
            AssetRegistrationProxy proxy = AssetRegistrationProxy();
            proxy.id = reg->id;
            proxy.relativePath = reg->regType == AssetRegistrationType::ASSET_REG_FILE
                ? gcnew System::String(reg->fileState->relativePath)
                : System::String::Empty;
            result[i] = proxy;
        }
        return result;
    }

    void EngineInterop::SetAssetData(uint32 assetId, System::IntPtr data, uint64 size)
    {
        memory->engineCode.api->assetsSetAssetData(
            memory->editor->engineMemory, assetId, data.ToPointer(), size);
    }

    void EngineInterop::InvalidateAsset(uint32 assetId)
    {
        memory->engineCode.api->assetsInvalidateAsset(memory->editor->engineMemory, assetId);
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

    void EngineInterop::Shutdown()
    {
        if (memory->editorCode.editorShutdown)
        {
            memory->editorCode.editorShutdown(memory->editor);
        }
    }

    uint32 EngineInterop::GetImportedHeightmapAssetId()
    {
        if (memory->editorCode.editorGetImportedHeightmapAssetId)
        {
            return memory->editorCode.editorGetImportedHeightmapAssetId(memory->editor);
        }

        return 0;
    }

    EditorToolProxy EngineInterop::GetBrushTool()
    {
        if (memory->editorCode.editorGetBrushTool)
        {
            return (EditorToolProxy)memory->editorCode.editorGetBrushTool(memory->editor);
        }
        return EditorToolProxy::RaiseTerrain;
    }

    void EngineInterop::SetBrushTool(EditorToolProxy tool)
    {
        if (memory->editorCode.editorSetBrushTool)
        {
            memory->editorCode.editorSetBrushTool(memory->editor, (EditorTool)tool);
        }
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

    void EngineInterop::SetBrushParameters(float radius, float falloff, float strength)
    {
        if (memory->editorCode.editorSetBrushParameters)
        {
            memory->editorCode.editorSetBrushParameters(
                memory->editor, radius, falloff, strength);
        }
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

    void EngineInterop::DeleteMaterial(int index)
    {
        if (memory->editorCode.editorDeleteMaterial)
        {
            memory->editorCode.editorDeleteMaterial(memory->editor, index);
        }
    }

    void EngineInterop::SwapMaterial(int indexA, int indexB)
    {
        if (memory->editorCode.editorSwapMaterial)
        {
            memory->editorCode.editorSwapMaterial(memory->editor, indexA, indexB);
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

    void EngineInterop::SetMaterialTexture(
        int index, TerrainMaterialTextureTypeProxy textureType, uint32 assetId)
    {
        if (memory->editorCode.editorSetMaterialTexture)
        {
            memory->editorCode.editorSetMaterialTexture(
                memory->editor, index, (TerrainMaterialTextureType)textureType, assetId);
        }
    }

    void EngineInterop::SetMaterialProperties(int index,
        float textureSize,
        float slopeStart,
        float slopeEnd,
        float altitudeStart,
        float altitudeEnd)
    {
        if (memory->editorCode.editorSetMaterialProperties)
        {
            memory->editorCode.editorSetMaterialProperties(memory->editor, index, textureSize,
                slopeStart, slopeEnd, altitudeStart, altitudeEnd);
        }
    }

    void EngineInterop::SetRockTransform(float positionX,
        float positionY,
        float positionZ,
        float rotationX,
        float rotationY,
        float rotationZ,
        float scaleX,
        float scaleY,
        float scaleZ)
    {
        if (memory->editorCode.editorSetRockTransform)
        {
            memory->editorCode.editorSetRockTransform(memory->editor, positionX, positionY,
                positionZ, rotationX, rotationY, rotationZ, scaleX, scaleY, scaleZ);
        }
    }

    void EngineInterop::SetSceneParameters(float lightDirection)
    {
        if (memory->editorCode.editorSetSceneParameters)
        {
            memory->editorCode.editorSetSceneParameters(memory->editor, lightDirection);
        }
    }
}}}