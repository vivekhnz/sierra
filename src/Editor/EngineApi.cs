using System;
using System.Runtime.InteropServices;

namespace Terrain.Editor
{
    enum AssetRegistrationType
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
        public uint Id;
        public AssetRegistrationType RegistrationType;
        public IntPtr StatePtr;
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

    delegate uint AssetsGetRegisteredAssetCount(IntPtr engineMemory);
    delegate ref AssetRegistration AssetsGetRegisteredAssets(IntPtr engineMemory);
    delegate void AssetsSetAssetData(IntPtr engineMemory, uint assetId, in byte data, ulong size);
    delegate void AssetsInvalidateAsset(IntPtr engineMemory, uint assetId);

    delegate bool RendererInitialize(IntPtr engineMemory, IntPtr getGlProcAddress);

    [StructLayout(LayoutKind.Sequential)]
    internal struct EngineApi
    {
        public AssetsGetRegisteredAssetCount assetsGetRegisteredAssetCount;
        public AssetsGetRegisteredAssets assetsGetRegisteredAssets;
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

        public RendererInitialize rendererInitialize;
        public IntPtr rendererUpdateCameraState;
        public IntPtr rendererUpdateLightingState;
        public IntPtr rendererCreateTexture;
        public IntPtr rendererBindTexture;
        public IntPtr rendererUpdateTexture;
        public IntPtr rendererReadTexturePixels;
        public IntPtr rendererCreateTextureArray;
        public IntPtr rendererBindTextureArray;
        public IntPtr rendererUpdateTextureArray;
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
        public IntPtr rendererDestroyResources;
    }
}
