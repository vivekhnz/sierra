using System;
using System.Runtime.InteropServices;

namespace Terrain.Editor
{
    internal enum AssetType
    {
        Shader = 1,
        ShaderProgram,
        Texture,
        Mesh
    }

    internal class Engine
    {
        private EngineApi api;
        private IntPtr memoryPtr;

        public Engine(EngineApi api, IntPtr memoryPtr)
        {
            this.api = api;
            this.memoryPtr = memoryPtr;
        }

        internal bool InitializeRenderer()
        {
            return api.rendererInitialize(memoryPtr, IntPtr.Zero);
        }

        internal int GetRegisteredAssetCount()
        {
            return (int)api.assetsGetRegisteredAssetCount(memoryPtr);
        }

        internal ReadOnlySpan<AssetRegistration> GetRegisteredAssets()
        {
            int assetCount = GetRegisteredAssetCount();
            ref AssetRegistration assetsRef = ref api.assetsGetRegisteredAssets(memoryPtr);
            return MemoryMarshal.CreateReadOnlySpan(ref assetsRef, assetCount);
        }

        internal void SetAssetData(uint assetId, ReadOnlySpan<byte> data)
        {
            api.assetsSetAssetData(memoryPtr, assetId, MemoryMarshal.AsRef<byte>(data), (ulong)data.Length);
        }

        internal void InvalidateAsset(uint assetId)
        {
            api.assetsInvalidateAsset(memoryPtr, assetId);
        }
    }

    internal static class EngineExtensions
    {
        internal static AssetType GetAssetType(this AssetRegistration assetReg)
        {
            return (AssetType)((assetReg.Id & 0xF0000000) >> 28);
        }

        internal static AssetFileState GetFileState(this AssetRegistration assetReg)
        {
            return Marshal.PtrToStructure<AssetFileState>(assetReg.StatePtr);
        }
    }
}
