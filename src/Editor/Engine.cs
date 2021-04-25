using System;
using System.Runtime.InteropServices;
using Terrain.Engine.Interop;

namespace Terrain.Editor
{
    internal enum AssetType
    {
        Shader = 1,
        ShaderProgram,
        Texture,
        Mesh
    }

    internal static class Engine
    {
        internal static uint GetRegisteredAssetCount()
        {
            return EngineInterop.GetRegisteredAssetCount();
        }

        internal static AssetRegistrationProxy[] GetRegisteredAssets()
        {
            return EngineInterop.GetRegisteredAssets();
        }

        internal static AssetType GetAssetType(AssetRegistrationProxy assetReg)
        {
            return (AssetType)((assetReg.id & 0xF0000000) >> 28);
        }

        internal static void SetAssetData(uint assetId, byte[] data)
        {
            GCHandle pinnedArray = GCHandle.Alloc(data, GCHandleType.Pinned);
            EngineInterop.SetAssetData(assetId, pinnedArray.AddrOfPinnedObject(), (ulong)data.Length);
            pinnedArray.Free();
        }

        internal static void InvalidateAsset(uint assetId)
        {
            EngineInterop.InvalidateAsset(assetId);
        }
    }
}
