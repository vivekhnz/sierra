#pragma once

#include "win32_editor_platform.h"
#include "Proxy/StateProxy.hpp"

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

        System::String ^ editorCodeDllPath;
        System::String ^ editorCodeDllShadowCopyPath;
        System::String ^ editorCodeBuildLockFilePath;

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
    ref class EngineInterop
    {
    private:
        static Win32PlatformMemory *memory = nullptr;
        static Proxy::StateProxy ^ stateProxy;

    public:
        static property Proxy::StateProxy^ State
        {
        public:
            Proxy::StateProxy^ get()
            {
                return stateProxy;
            }
        }

        static void InitializeEngine(EditorInitPlatformParamsProxy params);
        static void Shutdown();
        static void TickPlatform();

        static void Update(float deltaTime, EditorInputProxy input);
        static void RenderSceneView(EditorViewContextProxy % vctx);
        static void RenderHeightmapPreview(EditorViewContextProxy % vctx);
        static uint32 GetImportedHeightmapAssetId();

        static uint32 GetRegisteredAssetCount();
        static array<AssetRegistrationProxy> ^ GetRegisteredAssets();
        static void SetAssetData(uint32 assetId, System::IntPtr data, uint64 size);
        static void InvalidateAsset(uint32 assetId);

        static void AddMaterial(MaterialProps props);
        static void DeleteMaterial(int index);
        static void SwapMaterial(int indexA, int indexB);
        static void SetMaterialAlbedoTexture(int index, uint32 assetId);
        static void SetMaterialNormalTexture(int index, uint32 assetId);
        static void SetMaterialDisplacementTexture(int index, uint32 assetId);
        static void SetMaterialAoTexture(int index, uint32 assetId);
        static void SetMaterialTextureSize(int index, float value);
        static void SetMaterialSlopeStart(int index, float value);
        static void SetMaterialSlopeEnd(int index, float value);
        static void SetMaterialAltitudeStart(int index, float value);
        static void SetMaterialAltitudeEnd(int index, float value);
        static MaterialProps GetMaterialProperties(int index);
        static void SetRockTransform(float positionX,
            float positionY,
            float positionZ,
            float rotationX,
            float rotationY,
            float rotationZ,
            float scaleX,
            float scaleY,
            float scaleZ);
    };
}}}