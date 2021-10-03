using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;

namespace Terrain.Editor.Engine
{
    internal static class TerrainEngine
    {
        private delegate IntPtr EngineGetApi(
            IntPtr getGlProcAddress, EnginePlatformApi platformApi);

        private static IntPtr moduleHandle;

        private static Func<string, IntPtr> loadLibrary;
        private static Func<IntPtr, string, IntPtr> getProcAddress;
        private static Func<IntPtr, bool> freeLibrary;
        private static PlatformNotifyAssetRegistered onAssetRegistered
            = new PlatformNotifyAssetRegistered(TerrainEngine.OnAssetRegistered);
        private static EnginePlatformApi platformApi;

        private static EngineApi api;

        internal static IntPtr EngineApiPtr { get; private set; }

        internal delegate void AssetRegisteredEventHandler(in AssetRegistration assetReg);
        internal static event AssetRegisteredEventHandler AssetRegistered;

        internal static void Initialize(PlatformLogMessage logMessage,
            PlatformQueueAssetLoad queueAssetLoad, PlatformWatchAssetFile watchAssetFile,
            Func<string, IntPtr> loadLibrary, Func<IntPtr, string, IntPtr> getProcAddress,
            Func<IntPtr, bool> freeLibrary)
        {
            TerrainEngine.loadLibrary = loadLibrary;
            TerrainEngine.getProcAddress = getProcAddress;
            TerrainEngine.freeLibrary = freeLibrary;

            platformApi = new EnginePlatformApi
            {
                LogMessage = Marshal.GetFunctionPointerForDelegate(logMessage),
                QueueAssetLoad = Marshal.GetFunctionPointerForDelegate(queueAssetLoad),
                WatchAssetFile = Marshal.GetFunctionPointerForDelegate(watchAssetFile),
                NotifyAssetRegistered = Marshal.GetFunctionPointerForDelegate(onAssetRegistered)
            };
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

                EngineApiPtr = engineGetApi(IntPtr.Zero, platformApi);
                api = Marshal.PtrToStructure<EngineApi>(EngineApiPtr);
            }
        }

        private static void OnAssetRegistered(in AssetRegistration assetReg)
        {
            AssetRegistered?.Invoke(in assetReg);
        }

        internal static void SetAssetData(IntPtr assetHandle, ReadOnlySpan<byte> data)
        {
            api.assetsSetAssetData(
                assetHandle, MemoryMarshal.AsRef<byte>(data), (ulong)data.Length);
        }
    }

    internal static class EngineExtensions
    {
        internal static AssetFileState GetFileState(this AssetRegistration assetReg)
        {
            return Marshal.PtrToStructure<AssetFileState>(assetReg.StatePtr);
        }
    }
}
