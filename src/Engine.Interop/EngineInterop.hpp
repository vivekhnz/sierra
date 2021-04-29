#pragma once

#include "win32_editor_platform.h"

namespace Terrain { namespace Engine { namespace Interop {
public
    value struct MaterialProps
    {
        uint32 albedoTextureAssetId;
        uint32 normalTextureAssetId;
        uint32 displacementTextureAssetId;
        uint32 aoTextureAssetId;
        float textureSizeInWorldUnits;

        float slopeStart;
        float slopeEnd;
        float altitudeStart;
        float altitudeEnd;
    };

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
    value struct EditorInputProxy
    {
        System::IntPtr activeViewState;

        float scrollOffset;
        System::Windows::Point normalizedCursorPos;
        System::Windows::Vector cursorOffset;
        uint64 pressedButtons;
        uint64 prevPressedButtons;
    };

public
    value struct EditorViewContextProxy
    {
        System::IntPtr viewState;
        uint32 x;
        uint32 y;
        uint32 width;
        uint32 height;
    };

public
    value struct TerrainBrushParametersProxy
    {
        float Radius;
        float Falloff;
        float Strength;
    };

public
    ref class EngineInterop
    {
    private:
        static Win32PlatformMemory *memory = nullptr;

    public:
        // platform
        static void InitializeEngine(EditorInitPlatformParamsProxy params);
        static System::IntPtr ReloadEditorCode(
            System::String ^ dllPath, System::String ^ dllShadowCopyPath);

        // engine
        static System::IntPtr GetEngineMemory();

        // editor
        static System::IntPtr GetEditorMemory();
        static void SetEditorEngineApi(System::IntPtr engineApiPtr);
        static void Update(float deltaTime, EditorInputProxy input);
        static void RenderSceneView(EditorViewContextProxy % vctx);
        static void RenderHeightmapPreview(EditorViewContextProxy % vctx);

        static TerrainBrushParametersProxy GetBrushParameters();
        static void AddMaterial(MaterialProps props);
        static MaterialProps GetMaterialProperties(int index);
    };
}}}