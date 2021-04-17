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
        initParams.instance = (HINSTANCE)params.instance.ToPointer();
        initParams.mainWindowHwnd = (HWND)params.mainWindowHwnd.ToPointer();
        initParams.dummyWindowHwnd = (HWND)params.dummyWindowHwnd.ToPointer();
        initParams.glRenderingContext = (HGLRC)params.glRenderingContext.ToPointer();
        initParams.platformCaptureMouse =
            (PlatformCaptureMouse *)params.platformCaptureMouse.ToPointer();

        memory = win32InitializePlatform(&initParams);
        stateProxy = gcnew Proxy::StateProxy(&memory->editor->state.uiState);
    }

    void EngineInterop::Shutdown()
    {
        win32ShutdownPlatform();
    }

    void EngineInterop::TickApp(float deltaTime, EditorTickAppParamsProxy params)
    {
        Win32TickAppParams tickParams = {};
        tickParams.shouldCaptureMouse = params.shouldCaptureMouse;
        tickParams.wasMouseCaptured = params.wasMouseCaptured;
        tickParams.nextMouseScrollOffsetY = params.nextMouseScrollOffsetY;

        win32TickApp(deltaTime, &tickParams);
    }

    System::IntPtr EngineInterop::CreateViewportWindow(System::IntPtr deviceContextPtr,
        uint32 x,
        uint32 y,
        uint32 width,
        uint32 height,
        EditorView view)
    {
        Win32ViewportWindow *window = win32CreateViewportWindow(
            (HDC)deviceContextPtr.ToPointer(), x, y, width, height, view);
        return System::IntPtr(window);
    }

    void EngineInterop::ResizeViewportWindow(
        System::IntPtr windowPtr, uint32 x, uint32 y, uint32 width, uint32 height)
    {
        Win32ViewportWindow *window = (Win32ViewportWindow *)windowPtr.ToPointer();
        window->vctx.x = x;
        window->vctx.y = y;
        window->vctx.width = width;
        window->vctx.height = height;
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