using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;

namespace Terrain.Editor.Engine
{
    public enum AssetType
    {
        Shader = 1,
        ShaderProgram,
        Texture,
        Mesh
    }

    internal static class TerrainEngine
    {
        private delegate IntPtr EngineGetApi();

        private static EngineMemory memory;
        private static GCHandle memoryHandle;
        private static IntPtr memoryPtr;
        private static IntPtr moduleHandle;

        private static Func<string, IntPtr> loadLibrary;
        private static Func<IntPtr, string, IntPtr> getProcAddress;
        private static Func<IntPtr, bool> freeLibrary;
        private static PlatformNotifyAssetRegistered onAssetRegistered
            = new PlatformNotifyAssetRegistered(TerrainEngine.OnAssetRegistered);

        private static EngineApi api;

        internal static IntPtr EngineApiPtr { get; private set; }

        internal delegate void AssetRegisteredEventHandler(in AssetRegistration assetReg);
        internal static event AssetRegisteredEventHandler AssetRegistered;

        internal static IntPtr Initialize(IntPtr engineMemoryDataPtr,
            int engineMemorySizeInBytes, PlatformLogMessage logMessage,
            PlatformQueueAssetLoad queueAssetLoad, PlatformWatchAssetFile watchAssetFile,
            Func<string, IntPtr> loadLibrary, Func<IntPtr, string, IntPtr> getProcAddress,
            Func<IntPtr, bool> freeLibrary)
        {
            TerrainEngine.loadLibrary = loadLibrary;
            TerrainEngine.getProcAddress = getProcAddress;
            TerrainEngine.freeLibrary = freeLibrary;

            memory = new EngineMemory
            {
                PlatformLogMessage = Marshal.GetFunctionPointerForDelegate(logMessage),
                PlatformQueueAssetLoad = Marshal.GetFunctionPointerForDelegate(queueAssetLoad),
                PlatformWatchAssetFile = Marshal.GetFunctionPointerForDelegate(watchAssetFile),
                PlatformNotifyAssetRegistered = Marshal.GetFunctionPointerForDelegate(onAssetRegistered)
            };

            ulong offset = 0;
            memory.Renderer.BaseAddress = engineMemoryDataPtr + (int)offset;
            memory.Renderer.Size = 1 * 1024 * 1024;
            offset += memory.Renderer.Size;

            memory.Assets.BaseAddress = engineMemoryDataPtr + (int)offset;
            memory.Assets.Size = (ulong)engineMemorySizeInBytes - offset;
            offset += memory.Assets.Size;

            Debug.Assert(offset == (ulong)engineMemorySizeInBytes);

            memoryHandle = GCHandle.Alloc(memory, GCHandleType.Pinned);
            memoryPtr = memoryHandle.AddrOfPinnedObject();

            return memoryPtr;
        }

        internal static void ReloadCode(string dllPath, string dllShadowCopyPath)
        {
            if (moduleHandle != IntPtr.Zero)
            {
                freeLibrary(moduleHandle);
                moduleHandle = IntPtr.Zero;
                api = default(EngineApi);
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

            moduleHandle = loadLibrary(dllShadowCopyPath);
            if (moduleHandle != IntPtr.Zero)
            {
                IntPtr engineGetApiPtr = getProcAddress(moduleHandle, "engineGetApi");
                EngineGetApi engineGetApi = Marshal
                    .GetDelegateForFunctionPointer<EngineGetApi>(engineGetApiPtr);

                EngineApiPtr = engineGetApi();
                api = Marshal.PtrToStructure<EngineApi>(EngineApiPtr);
                api.rendererInitialize(ref memory, IntPtr.Zero);
            }
        }

        internal static void Shutdown()
        {
            if (moduleHandle != IntPtr.Zero)
            {
                api.rendererDestroyResources(ref memory);
            }
            memoryHandle.Free();
        }

        private static void OnAssetRegistered(in AssetRegistration assetReg)
        {
            AssetRegistered?.Invoke(in assetReg);
        }

        internal static void SetAssetData(uint assetId, ReadOnlySpan<byte> data)
        {
            api.assetsSetAssetData(ref memory, assetId, MemoryMarshal.AsRef<byte>(data), (ulong)data.Length);
        }

        internal static void InvalidateAsset(uint assetId)
        {
            api.assetsInvalidateAsset(ref memory, assetId);
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
