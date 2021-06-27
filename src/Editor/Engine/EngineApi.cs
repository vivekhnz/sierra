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

    public enum AssetRegistrationType
    {
        File,
        Composite,
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
        public IntPtr assetsRegisterShaderProgram;
        public IntPtr assetsRegisterMesh;
        public IntPtr assetsGetShader;
        public IntPtr assetsGetShaderProgram;
        public IntPtr assetsGetTexture;
        public IntPtr assetsGetMesh;
        public AssetsSetAssetData assetsSetAssetData;
        public AssetsInvalidateAsset assetsInvalidateAsset;

        public IntPtr heightfieldGetHeight;
        public IntPtr heightfieldIsRayIntersecting;

        public IntPtr rendererInitialize;
        public IntPtr rendererUpdateCameraState;
        public IntPtr rendererUpdateLightingState;
        public IntPtr rendererCreateTexture;
        public IntPtr rendererBindTexture;
        public IntPtr rendererUpdateTexture;
        public IntPtr rendererReadTexturePixels;
        public IntPtr rendererCreateTextureArray;
        public IntPtr rendererBindTextureArray;
        public IntPtr rendererUpdateTextureArray;
        public IntPtr rendererCreateDepthBuffer;
        public IntPtr rendererResizeDepthBuffer;
        public IntPtr rendererCreateFramebuffer;
        public IntPtr rendererBindFramebuffer;
        public IntPtr rendererUnbindFramebuffer;
        public IntPtr rendererCreateShader;
        public IntPtr rendererCreateShaderProgram;
        public IntPtr rendererUseShaderProgram;
        public IntPtr rendererSetShaderProgramUniformFloat;
        public IntPtr rendererSetShaderProgramUniformInteger;
        public IntPtr rendererSetShaderProgramUniformVector2;
        public IntPtr rendererSetShaderProgramUniformVector3;
        public IntPtr rendererSetShaderProgramUniformVector4;
        public IntPtr rendererSetShaderProgramUniformMatrix4x4;
        public IntPtr rendererCreateVertexArray;
        public IntPtr rendererBindVertexArray;
        public IntPtr rendererUnbindVertexArray;
        public IntPtr rendererCreateBuffer;
        public IntPtr rendererBindBuffer;
        public IntPtr rendererUpdateBuffer;
        public IntPtr rendererBindVertexAttribute;
        public IntPtr rendererBindShaderStorageBuffer;
        public IntPtr rendererSetViewportSize;
        public IntPtr rendererClearBackBuffer;
        public IntPtr rendererSetPolygonMode;
        public IntPtr rendererSetBlendMode;
        public IntPtr rendererDrawElements;
        public IntPtr rendererDrawElementsInstanced;
        public IntPtr rendererDispatchCompute;
        public IntPtr rendererShaderStorageMemoryBarrier;
        public IntPtr rendererCreateRenderTarget;
        public IntPtr rendererCreateQueue;
        public IntPtr rendererSetCamera;
        public IntPtr rendererClear;
        public IntPtr rendererPushTexturedQuad;
        public IntPtr rendererDrawToTarget;
        public IntPtr rendererDrawToScreen;
    }
}
