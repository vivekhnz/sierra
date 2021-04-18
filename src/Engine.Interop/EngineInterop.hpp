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
    };

public
    value struct EditorTickAppParamsProxy
    {
        System::IntPtr mainWindowHwnd;
        System::IntPtr glRenderingContext;

        bool shouldCaptureMouse;
        bool wasMouseCaptured;
        float nextMouseScrollOffsetY;
        uint64 pressedButtons;
        uint64 prevPressedButtons;
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
        static void TickApp(float deltaTime, EditorTickAppParamsProxy params);

        static System::IntPtr CreateViewportWindow(System::IntPtr deviceContextPtr,
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