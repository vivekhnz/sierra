#pragma once

#include <vector>

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
    value struct EditorPlatformViewportWindow
    {
        System::IntPtr windowPtr;
        System::IntPtr windowHwnd;
    };

public
    ref class EngineInterop
    {
    private:
        static Win32PlatformMemory *memory = nullptr;
        static Proxy::StateProxy ^ stateProxy;

        static System::Windows::Threading::DispatcherTimer ^ renderTimer = nullptr;
        static System::DateTime lastTickTime;

        static void OnTick(Object ^ sender, System::EventArgs ^ e);

    public:
        static property Proxy::StateProxy^ State
        {
        public:
            Proxy::StateProxy^ get()
            {
                return stateProxy;
            }
        }

        static void InitializeEngine();
        static void Shutdown();

        static EditorPlatformViewportWindow CreateViewportWindow(System::IntPtr parentHwnd,
            uint32 x,
            uint32 y,
            uint32 width,
            uint32 height,
            EditorView view);
        static void ResizeViewportWindow(
            System::IntPtr windowPtr, uint32 x, uint32 y, uint32 width, uint32 height);

        static void LoadHeightmapTexture(System::String ^ path);
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