using System;
using System.Runtime.InteropServices;

namespace Sierra.Core
{
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