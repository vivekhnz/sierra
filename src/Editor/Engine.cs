using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;

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
        private delegate IntPtr EngineGetApi();

        private static IntPtr memoryPtr;
        private static IntPtr moduleHandle;

        private static EngineApi api;

        internal static IntPtr EngineApiPtr { get; private set; }

        internal static void Initialize(IntPtr engineMemoryPtr)
        {
            memoryPtr = engineMemoryPtr;
        }

        internal static void ReloadCode(string dllPath, string dllShadowCopyPath)
        {
            if (moduleHandle != IntPtr.Zero)
            {
                Win32.FreeLibrary(moduleHandle);
                moduleHandle = IntPtr.Zero;
            }

            bool didShadowCopySucceed = false;
            while (!didShadowCopySucceed)
            {
                try
                {
                    File.Copy(dllPath, dllShadowCopyPath, true);
                    didShadowCopySucceed = true;
                }
                catch
                {
                    Thread.Sleep(100);
                }
            }

            moduleHandle = Win32.LoadLibrary(dllShadowCopyPath);
            if (moduleHandle != IntPtr.Zero)
            {
                IntPtr engineGetApiPtr = Win32.GetProcAddress(
                    moduleHandle, "engineGetApi");
                EngineGetApi engineGetApi = Marshal
                    .GetDelegateForFunctionPointer<EngineGetApi>(engineGetApiPtr);

                EngineApiPtr = engineGetApi();
                api = Marshal.PtrToStructure<EngineApi>(EngineApiPtr);
                api.rendererInitialize(memoryPtr, IntPtr.Zero);
            }
        }

        internal static int GetRegisteredAssetCount()
        {
            return (int)api.assetsGetRegisteredAssetCount(memoryPtr);
        }

        internal static ReadOnlySpan<AssetRegistration> GetRegisteredAssets()
        {
            int assetCount = GetRegisteredAssetCount();
            ref AssetRegistration assetsRef = ref api.assetsGetRegisteredAssets(memoryPtr);
            return MemoryMarshal.CreateReadOnlySpan(ref assetsRef, assetCount);
        }

        internal static void SetAssetData(uint assetId, ReadOnlySpan<byte> data)
        {
            api.assetsSetAssetData(memoryPtr, assetId, MemoryMarshal.AsRef<byte>(data), (ulong)data.Length);
        }

        internal static void InvalidateAsset(uint assetId)
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
