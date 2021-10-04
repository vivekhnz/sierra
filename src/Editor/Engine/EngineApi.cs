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
    internal delegate long PlatformGetFileLastWriteTime(string relativePath);
    internal delegate long PlatformGetFileSize(string path);
    internal delegate void PlatformReadEntireFile(string path, ref byte bufferBaseAddress);
    internal delegate void PlatformNotifyAssetRegistered(in AssetRegistration assetReg);

    [StructLayout(LayoutKind.Sequential)]
    struct EnginePlatformApi
    {
        public IntPtr LogMessage;
        public IntPtr GetFileLastWriteTime;
        public IntPtr GetFileSize;
        public IntPtr ReadEntireFile;
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
        public long LastWriteTime;
    }
}
