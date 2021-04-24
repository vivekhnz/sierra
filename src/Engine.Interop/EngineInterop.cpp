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
    }

    void EngineInterop::InitializeEngine(EditorInitPlatformParamsProxy params)
    {
        Win32InitPlatformParams initParams = {};
        initParams.memoryBaseAddress = (uint8 *)params.memoryPtr.ToPointer();
        initParams.editorMemorySize = params.editorMemorySize;
        initParams.engineMemorySize = params.engineMemorySize;
        GetCStringFromManagedString(params.editorCodeDllPath, initParams.editorCodeDllPath);
        GetCStringFromManagedString(
            params.editorCodeDllShadowCopyPath, initParams.editorCodeDllShadowCopyPath);
        GetCStringFromManagedString(
            params.editorCodeBuildLockFilePath, initParams.editorCodeBuildLockFilePath);
        initParams.platformCaptureMouse =
            (PlatformCaptureMouse *)params.platformCaptureMouse.ToPointer();

        memory = win32InitializePlatform(&initParams);
        stateProxy = gcnew Proxy::StateProxy(&memory->editor->state.uiState);
    }

    void EngineInterop::Shutdown()
    {
        win32ShutdownPlatform();
    }

    void EngineInterop::TickApp(float deltaTime, EditorInputProxy input)
    {
        EditorInput inputInternal = {};
        inputInternal.activeViewState = input.activeViewState.ToPointer();
        inputInternal.scrollOffset = input.scrollOffset;
        inputInternal.normalizedCursorPos =
            glm::vec2(input.normalizedCursorPos.X, input.normalizedCursorPos.Y);
        inputInternal.cursorOffset = glm::vec2(input.cursorOffset.X, input.cursorOffset.Y);
        inputInternal.pressedButtons = input.pressedButtons;
        inputInternal.prevPressedButtons = input.prevPressedButtons;

        win32TickApp(deltaTime, &inputInternal);
    }

    void EngineInterop::RenderSceneView(EditorViewContextProxy % vctx)
    {
        EditorViewContext vctxInternal;
        vctxInternal.viewState = vctx.viewState.ToPointer();
        vctxInternal.x = vctx.x;
        vctxInternal.y = vctx.y;
        vctxInternal.width = vctx.width;
        vctxInternal.height = vctx.height;

        win32RenderSceneView(&vctxInternal);

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

        win32RenderHeightmapPreview(&vctxInternal);

        vctx.viewState = System::IntPtr(vctxInternal.viewState);
    }

    uint32 EngineInterop::GetRegisteredAssetCount()
    {
        return memory->engineApi.assetsGetRegisteredAssetCount(memory->editor->engineMemory);
    }

    array<AssetRegistrationProxy> ^ EngineInterop::GetRegisteredAssets()
    {
        uint32 assetCount = GetRegisteredAssetCount();
        AssetRegistration *assetRegs =
            memory->engineApi.assetsGetRegisteredAssets(memory->editor->engineMemory);

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

    void EngineInterop::LoadHeightmapTexture(System::String ^ path)
    {
        GetCStringFromManagedString(path, memory->importedHeightmapTexturePath);
    }

    void EngineInterop::AddMaterial(MaterialProps props)
    {
        assert(memory->editor->state.uiState.materialCount < MAX_MATERIAL_COUNT);
        uint32 index = memory->editor->state.uiState.materialCount++;

        MaterialProperties *material = &memory->editor->state.uiState.materialProps[index];
        material->albedoTextureAssetId = props.albedoTextureAssetId;
        material->normalTextureAssetId = props.normalTextureAssetId;
        material->displacementTextureAssetId = props.displacementTextureAssetId;
        material->aoTextureAssetId = props.aoTextureAssetId;
        material->textureSizeInWorldUnits = props.textureSizeInWorldUnits;
        material->slopeStart = props.slopeStart;
        material->slopeEnd = props.slopeEnd;
        material->altitudeStart = props.altitudeStart;
        material->altitudeEnd = props.altitudeEnd;
    }

    void EngineInterop::DeleteMaterial(int index)
    {
        assert(index < MAX_MATERIAL_COUNT);

        memory->editor->state.uiState.materialCount--;
        for (uint32 i = index; i < memory->editor->state.uiState.materialCount; i++)
        {
            memory->editor->state.uiState.materialProps[i] =
                memory->editor->state.uiState.materialProps[i + 1];
        }
    }

    void EngineInterop::SwapMaterial(int indexA, int indexB)
    {
        assert(indexA < MAX_MATERIAL_COUNT);
        assert(indexB < MAX_MATERIAL_COUNT);

        MaterialProperties temp = memory->editor->state.uiState.materialProps[indexA];
        memory->editor->state.uiState.materialProps[indexA] =
            memory->editor->state.uiState.materialProps[indexB];
        memory->editor->state.uiState.materialProps[indexB] = temp;
    }

    void EngineInterop::SetMaterialAlbedoTexture(int index, uint32 assetId)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor->state.uiState.materialProps[index].albedoTextureAssetId = assetId;
    }

    void EngineInterop::SetMaterialNormalTexture(int index, uint32 assetId)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor->state.uiState.materialProps[index].normalTextureAssetId = assetId;
    }

    void EngineInterop::SetMaterialDisplacementTexture(int index, uint32 assetId)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor->state.uiState.materialProps[index].displacementTextureAssetId =
            assetId;
    }

    void EngineInterop::SetMaterialAoTexture(int index, uint32 assetId)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor->state.uiState.materialProps[index].aoTextureAssetId = assetId;
    }

    void EngineInterop::SetMaterialTextureSize(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor->state.uiState.materialProps[index].textureSizeInWorldUnits = value;
    }

    void EngineInterop::SetMaterialSlopeStart(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor->state.uiState.materialProps[index].slopeStart = value;
    }

    void EngineInterop::SetMaterialSlopeEnd(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor->state.uiState.materialProps[index].slopeEnd = value;
    }

    void EngineInterop::SetMaterialAltitudeStart(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor->state.uiState.materialProps[index].altitudeStart = value;
    }

    void EngineInterop::SetMaterialAltitudeEnd(int index, float value)
    {
        assert(index < MAX_MATERIAL_COUNT);
        memory->editor->state.uiState.materialProps[index].altitudeEnd = value;
    }

    MaterialProps EngineInterop::GetMaterialProperties(int index)
    {
        assert(index < MAX_MATERIAL_COUNT);
        MaterialProperties *state = &memory->editor->state.uiState.materialProps[index];

        MaterialProps result = {};

        result.albedoTextureAssetId = state->albedoTextureAssetId;
        result.normalTextureAssetId = state->normalTextureAssetId;
        result.displacementTextureAssetId = state->displacementTextureAssetId;
        result.aoTextureAssetId = state->aoTextureAssetId;
        result.textureSizeInWorldUnits = state->textureSizeInWorldUnits;

        result.slopeStart = state->slopeStart;
        result.slopeEnd = state->slopeEnd;
        result.altitudeStart = state->altitudeStart;
        result.altitudeEnd = state->altitudeEnd;

        return result;
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
        memory->editor->state.uiState.rockPosition.x = positionX;
        memory->editor->state.uiState.rockPosition.y = positionY;
        memory->editor->state.uiState.rockPosition.z = positionZ;
        memory->editor->state.uiState.rockRotation.x = rotationX;
        memory->editor->state.uiState.rockRotation.y = rotationY;
        memory->editor->state.uiState.rockRotation.z = rotationZ;
        memory->editor->state.uiState.rockScale.x = scaleX;
        memory->editor->state.uiState.rockScale.y = scaleY;
        memory->editor->state.uiState.rockScale.z = scaleZ;
    }
}}}