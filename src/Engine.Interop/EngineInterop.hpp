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

        System::String ^ buildLockFilePath;
        System::String ^ engineCodeDllPath;
        System::String ^ engineCodeDllShadowCopyPath;

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
    value struct AssetRegistrationProxy
    {
        uint32 id;
        System::String ^ relativePath;
    };

public
    enum class EditorToolProxy
    {
        RaiseTerrain = 0,
        LowerTerrain = 1,
        FlattenTerrain = 2,
        SmoothTerrain = 3
    };

public
    value struct TerrainBrushParameters
    {
        float Radius;
        float Falloff;
        float Strength;
    };

public
    enum class TerrainMaterialTextureType
    {
        Albedo = 0,
        Normal = 1,
        Displacement = 2,
        AmbientOcclusion = 3
    };

public
    ref class EngineInterop
    {
    private:
        static Win32PlatformMemory *memory = nullptr;

    public:
        // platform
        static void InitializeEngine(EditorInitPlatformParamsProxy params);
        static void TickPlatform(System::String^ engineCodeDllPath,
            System::String^ engineCodeDllShadowCopyPath,
            System::String^ editorCodeDllPath,
            System::String^ editorCodeDllShadowCopyPath);

        // engine
        static uint32 GetRegisteredAssetCount();
        static array<AssetRegistrationProxy> ^ GetRegisteredAssets();
        static void SetAssetData(uint32 assetId, System::IntPtr data, uint64 size);
        static void InvalidateAsset(uint32 assetId);

        // editor
        static void Update(float deltaTime, EditorInputProxy input);
        static void RenderSceneView(EditorViewContextProxy % vctx);
        static void RenderHeightmapPreview(EditorViewContextProxy % vctx);
        static void Shutdown();

        static uint32 GetImportedHeightmapAssetId();
        static EditorToolProxy GetBrushTool();
        static void SetBrushTool(EditorToolProxy tool);
        static TerrainBrushParameters GetBrushParameters();
        static void SetBrushParameters(float radius, float falloff, float strength);
        static void AddMaterial(MaterialProps props);
        static void DeleteMaterial(int index);
        static void SwapMaterial(int indexA, int indexB);
        static MaterialProps GetMaterialProperties(int index);
        static void SetMaterialTexture(
            int index, TerrainMaterialTextureType textureType, uint32 assetId);
        static void SetMaterialParameters(int index,
            float textureSize,
            float slopeStart,
            float slopeEnd,
            float altitudeStart,
            float altitudeEnd);
        static void SetRockTransform(float positionX,
            float positionY,
            float positionZ,
            float rotationX,
            float rotationY,
            float rotationZ,
            float scaleX,
            float scaleY,
            float scaleZ);
        static void SetSceneParameters(float lightDirection);
    };
}}}