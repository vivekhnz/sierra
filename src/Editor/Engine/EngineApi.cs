using System;
using System.Runtime.InteropServices;

namespace Terrain.Editor.Engine
{
    [StructLayout(LayoutKind.Sequential)]
    struct MemoryArena
    {
        public IntPtr BaseAddress;
        public ulong Size;
        public ulong Used;
    }

    internal delegate void PlatformLogMessage(string message);
    internal delegate bool PlatformQueueAssetLoad(IntPtr assetHandle, string relativePath);
    internal delegate void PlatformWatchAssetFile(IntPtr assetHandle, string relativePath);
    internal delegate void PlatformNotifyAssetRegistered(in AssetRegistration assetReg);

    [StructLayout(LayoutKind.Sequential)]
    struct EnginePlatformApi
    {
        public IntPtr LogMessage;
        public IntPtr QueueAssetLoad;
        public IntPtr WatchAssetFile;
        public IntPtr NotifyAssetRegistered;
    }

    public enum AssetType
    {
        Shader = 1,
        Texture,
        Mesh
    }

    public enum AssetRegistrationType
    {
        File,
        Virtual
    }

    [StructLayout(LayoutKind.Sequential)]
    struct LoadedAsset
    {
        public byte Version;
        public IntPtr AssetData;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct AssetRegistration
    {
        public IntPtr Handle;
        public AssetRegistrationType RegistrationType;
        public IntPtr StatePtr;
        public AssetType AssetType;
        public IntPtr MetadataPtr;
        public LoadedAsset Asset;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct AssetFileState
    {
        public string RelativePath;
        public bool IsUpToDate;
        public bool IsLoadQueued;
    }

    delegate void AssetsSetAssetData(IntPtr assetHandle, in byte data, ulong size);
    delegate void AssetsInvalidateAsset(IntPtr assetHandle);

    [StructLayout(LayoutKind.Sequential)]
    internal struct EngineApi
    {
        public IntPtr assetsInitialize;
        public IntPtr assetsRegisterShader;
        public IntPtr assetsRegisterTexture;
        public IntPtr assetsRegisterMesh;
        public IntPtr assetsGetShader;
        public IntPtr assetsGetTexture;
        public IntPtr assetsGetMesh;
        public AssetsSetAssetData assetsSetAssetData;
        public AssetsInvalidateAsset assetsInvalidateAsset;

        public IntPtr heightfieldGetHeight;
        public IntPtr heightfieldIsRayIntersecting;

        public IntPtr rendererInitialize;
        public IntPtr rendererUpdateLightingState;
        public IntPtr rendererCreateTexture;
        public IntPtr rendererUpdateTexture;
        public IntPtr rendererReadTexturePixels;
        public IntPtr rendererCreateTextureArray;
        public IntPtr rendererUpdateTextureArray;
        public IntPtr rendererCreateBuffer;
        public IntPtr rendererUpdateBuffer;
        public IntPtr rendererCreateRenderTarget;
        public IntPtr rendererResizeRenderTarget;
        public IntPtr rendererGetPixels;
        public IntPtr rendererCreateEffect;
        public IntPtr rendererSetEffectFloat;
        public IntPtr rendererSetEffectInt;
        public IntPtr rendererSetEffectUint;
        public IntPtr rendererSetEffectTexture;
        public IntPtr rendererCreateQueue;
        public IntPtr rendererSetCameraOrtho;
        public IntPtr rendererSetCameraPersp;
        public IntPtr rendererClear;
        public IntPtr rendererPushTexturedQuad;
        public IntPtr rendererPushQuad;
        public IntPtr rendererPushQuads;
        public IntPtr rendererPushMeshes;
        public IntPtr rendererPushTerrain;
        public IntPtr rendererDrawToTarget;
        public IntPtr rendererDrawToScreen;
    }
}
