﻿using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Sierra.Core
{
    internal delegate void PlatformLogMessage(string message);
    internal delegate long PlatformGetFileLastWriteTime(string relativePath);
    internal delegate long PlatformGetFileSize(string path);
    internal delegate void PlatformReadEntireFile(string path, ref byte bufferBaseAddress);
    internal delegate void PlatformWriteEntireFile(string path, ref byte bufferBaseAddress, ulong size);
    internal delegate void PlatformPublishTransaction(ref byte commandBufferBaseAddress);
    internal delegate void PlatformNotifyAssetRegistered(in AssetRegistration assetReg);
    internal delegate void PlatformStartPerfCounter(string counterName);
    internal delegate void PlatformEndPerfCounter(string counterName);

    internal struct EditorPlatformApi
    {
        public IntPtr LogMessage;
        public IntPtr GetFileLastWriteTime;
        public IntPtr GetFileSize;
        public IntPtr ReadEntireFile;
        public IntPtr WriteEntireFile;
        public IntPtr NotifyAssetRegistered;
        public IntPtr PublishTransaction;
        public IntPtr StartPerfCounter;
        public IntPtr EndPerfCounter;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct MemoryArena
    {
        public IntPtr BaseAddress;
        public ulong Size;
        public ulong Used;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct EditorMemory
    {
        public MemoryArena Data;
        public EditorPlatformApi PlatformApi;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct EditorInput
    {
        public static readonly EditorInput Disabled = new EditorInput();

        public bool IsActive;
        public float ScrollOffset;
        public Vector2 CursorPos;
        public ulong PressedButtons;
        public ulong PreviousPressedButtons;
        public Vector2 CapturedCursorDelta;

        public bool IsMouseCaptured;
    }

    [Flags]
    internal enum EditorInputButtons : ulong
    {
        MouseLeft = 1L << 0,
        MouseMiddle = 1L << 1,
        MouseRight = 1L << 2,
        KeySpace = 1L << 3,
        Key0 = 1L << 4,
        Key1 = 1L << 5,
        Key2 = 1L << 6,
        Key3 = 1L << 7,
        Key4 = 1L << 8,
        Key5 = 1L << 9,
        Key6 = 1L << 10,
        Key7 = 1L << 11,
        Key8 = 1L << 12,
        Key9 = 1L << 13,
        KeyA = 1L << 14,
        KeyB = 1L << 15,
        KeyC = 1L << 16,
        KeyD = 1L << 17,
        KeyE = 1L << 18,
        KeyF = 1L << 19,
        KeyG = 1L << 20,
        KeyH = 1L << 21,
        KeyI = 1L << 22,
        KeyJ = 1L << 23,
        KeyK = 1L << 24,
        KeyL = 1L << 25,
        KeyM = 1L << 26,
        KeyN = 1L << 27,
        KeyO = 1L << 28,
        KeyP = 1L << 29,
        KeyQ = 1L << 30,
        KeyR = 1L << 31,
        KeyS = 1L << 32,
        KeyT = 1L << 33,
        KeyU = 1L << 34,
        KeyV = 1L << 35,
        KeyW = 1L << 36,
        KeyX = 1L << 37,
        KeyY = 1L << 38,
        KeyZ = 1L << 39,
        KeyEscape = 1L << 40,
        KeyEnter = 1L << 41,
        KeyRight = 1L << 42,
        KeyLeft = 1L << 43,
        KeyDown = 1L << 44,
        KeyUp = 1L << 45,
        KeyF1 = 1L << 46,
        KeyF2 = 1L << 47,
        KeyF3 = 1L << 48,
        KeyF4 = 1L << 49,
        KeyF5 = 1L << 50,
        KeyF6 = 1L << 51,
        KeyF7 = 1L << 52,
        KeyF8 = 1L << 53,
        KeyF9 = 1L << 54,
        KeyF10 = 1L << 55,
        KeyF11 = 1L << 56,
        KeyF12 = 1L << 57,
        KeyLeftShift = 1L << 58,
        KeyLeftControl = 1L << 59,
        KeyAlt = 1L << 60,
        KeyDelete = 1L << 61
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct EditorViewContext
    {
        public IntPtr ViewState;
        public uint Width;
        public uint Height;
    }

    public enum EditorContext
    {
        Terrain = 0,
        Objects = 1,
        Scene = 2
    };

    public enum TerrainBrushTool
    {
        Raise = 0,
        Lower = 1,
        Flatten = 2,
        Smooth = 3
    }

    [StructLayout(LayoutKind.Sequential)]
    internal unsafe struct EditorUiState
    {
        private const int MaxSelectedObjects = 32;

        public EditorContext CurrentContext;

        private uint* _selectedObjectIds;
        public uint SelectedObjectCount;
        public Span<uint> SelectedObjectIds
            => new Span<uint>(_selectedObjectIds, MaxSelectedObjects);

        public TerrainBrushTool TerrainBrushTool;
        public float TerrainBrushRadius;
        public float TerrainBrushFalloff;
        public float TerrainBrushStrength;

        public float SceneLightDirection;

        public EditorDebugState DebugState;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct EditorDebugState
    {
        public byte ShowTerrainRaycastVis;
        public byte ShowTerrainTileBounds;
        public byte ShowTerrainTileHeightmap;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct TerrainMaterialProperties
    {
        public IntPtr AlbedoTextureAssetHandle;
        public IntPtr NormalTextureAssetHandle;
        public IntPtr DisplacementTextureAssetHandle;
        public IntPtr AoTextureAssetHandle;
        public float TextureSizeInWorldUnits;

        public float SlopeStart;
        public float SlopeEnd;
        public float AltitudeStart;
        public float AltitudeEnd;
    }

    internal enum TerrainMaterialTextureType
    {
        Albedo = 0,
        Normal = 1,
        Displacement = 2,
        AmbientOcclusion = 3
    }

    internal enum EditorCommandType
    {
        AddMaterial,
        DeleteMaterial,
        SwapMaterial,
        SetMaterialTexture,
        SetMaterialProperties,
        AddObject,
        DeleteObject,
        SetObjectProperty
    }

    internal enum ObjectProperty
    {
        ObjectPositionX,
        ObjectPositionY,
        ObjectPositionZ,
        ObjectRotationX,
        ObjectRotationY,
        ObjectRotationZ,
        ObjectScaleX,
        ObjectScaleY,
        ObjectScaleZ,
    }

    [StructLayout(LayoutKind.Sequential)]
    struct AddMaterialCommand
    {
        public uint MaterialId;

        public IntPtr AlbedoTextureAssetHandle;
        public IntPtr NormalTextureAssetHandle;
        public IntPtr DisplacementTextureAssetHandle;
        public IntPtr AoTextureAssetHandle;
        public float TextureSizeInWorldUnits;

        public float SlopeStart;
        public float SlopeEnd;
        public float AltitudeStart;
        public float AltitudeEnd;
    }
    [StructLayout(LayoutKind.Sequential)]
    struct DeleteMaterialCommand
    {
        public uint Index;
    }
    [StructLayout(LayoutKind.Sequential)]
    struct SwapMaterialCommand
    {
        public uint IndexA;
        public uint IndexB;
    }
    [StructLayout(LayoutKind.Sequential)]
    struct SetMaterialTextureCommand
    {
        public uint Index;
        public TerrainMaterialTextureType TextureType;
        public IntPtr AssetHandle;
    }
    [StructLayout(LayoutKind.Sequential)]
    struct SetMaterialPropertiesCommand
    {
        public uint MaterialId;
        public float TextureSizeInWorldUnits;
        public float SlopeStart;
        public float SlopeEnd;
        public float AltitudeStart;
        public float AltitudeEnd;
    }
    [StructLayout(LayoutKind.Sequential)]
    struct AddObjectCommand
    {
        public uint ObjectId;
    }
    [StructLayout(LayoutKind.Sequential)]
    struct DeleteObjectCommand
    {
        public uint ObjectId;
    }
    [StructLayout(LayoutKind.Sequential)]
    struct SetObjectPropertyCommand
    {
        public uint ObjectId;
        public ObjectProperty Property;
        public float Value;
    }

    internal struct Transaction
    {
        internal static Transaction Invalid { get; } = new Transaction(IntPtr.Zero);

        internal readonly IntPtr Pointer;
        internal bool IsValid => Pointer != IntPtr.Zero;

        internal Transaction(IntPtr ptr)
        {
            Pointer = ptr;
        }
    }

    internal static class EditorCore
    {
        private static object reloadLock = new object();
        private static IntPtr appMemoryDataPtr;
        private static IntPtr moduleHandle;

        private static Func<string, IntPtr> loadLibrary;
        private static Func<IntPtr, string, IntPtr> getProcAddress;
        private static Func<IntPtr, bool> freeLibrary;

        private delegate void TransactionPublishedCallback(ref byte commandBufferBaseAddress);
        private static TransactionPublishedCallback onTransactionPublished
            = new TransactionPublishedCallback(OnTransactionPublished);

        private static PlatformNotifyAssetRegistered onAssetRegistered
            = new PlatformNotifyAssetRegistered(EditorCore.OnAssetRegistered);
        internal delegate void AssetRegisteredEventHandler(in AssetRegistration assetReg);
        internal static event AssetRegisteredEventHandler AssetRegistered;

        private static EditorUiState defaultEditorState = default(EditorUiState);

        delegate void EditorUpdate(ref EditorMemory memory, float deltaTime);
        delegate void EditorRenderSceneView(ref EditorMemory memory, ref EditorViewContext view,
            float deltaTime, ref EditorInput input);
        delegate void EditorRenderHeightmapPreview(ref EditorMemory memory, ref EditorViewContext view,
            float deltaTime, ref EditorInput input);
        delegate IntPtr EditorGetImportedHeightmapAssetHandle(ref EditorMemory memory);
        delegate void EditorSaveHeightmap(ref EditorMemory memory, string filePath);
        delegate ref EditorUiState EditorGetUiState(ref EditorMemory memory);
        delegate void EditorAddMaterial(ref EditorMemory memory, TerrainMaterialProperties props);
        delegate void EditorDeleteMaterial(ref EditorMemory memory, uint index);
        delegate void EditorSwapMaterial(ref EditorMemory memory, uint indexA, uint indexB);
        delegate void EditorSetMaterialTexture(ref EditorMemory memory, uint materialId,
            TerrainMaterialTextureType textureType, IntPtr assetHandle);
        delegate void EditorSetMaterialProperties(ref EditorMemory memory, uint materialId, float textureSize,
            float slopeStart, float slopeEnd, float altitudeStart, float altitudeEnd);
        delegate float EditorGetObjectProperty(ref EditorMemory memory, uint objectId,
            ObjectProperty property);
        delegate IntPtr EditorBeginTransaction(ref EditorMemory memory);
        delegate void EditorClearTransaction(IntPtr tx);
        delegate void EditorCommitTransaction(IntPtr tx);
        delegate void EditorDiscardTransaction(IntPtr tx);
        delegate void EditorAddObject(ref EditorMemory memory, IntPtr tx);
        delegate void EditorDeleteObject(IntPtr tx, uint objectId);
        delegate void EditorSetObjectProperty(IntPtr tx, uint objectId,
            ObjectProperty property, float value);

        private static EditorUpdate editorUpdate;
        private static EditorRenderSceneView editorRenderSceneView;
        private static EditorRenderHeightmapPreview editorRenderHeightmapPreview;
        private static EditorGetImportedHeightmapAssetHandle editorGetImportedHeightmapAssetHandle;
        private static EditorSaveHeightmap editorSaveHeightmap;
        private static EditorGetUiState editorGetUiState;
        private static EditorAddMaterial editorAddMaterial;
        private static EditorDeleteMaterial editorDeleteMaterial;
        private static EditorSwapMaterial editorSwapMaterial;
        private static EditorSetMaterialTexture editorSetMaterialTexture;
        private static EditorSetMaterialProperties editorSetMaterialProperties;
        private static EditorGetObjectProperty editorGetObjectProperty;
        private static EditorBeginTransaction editorBeginTransaction;
        private static EditorClearTransaction editorClearTransaction;
        private static EditorCommitTransaction editorCommitTransaction;
        private static EditorDiscardTransaction editorDiscardTransaction;
        private static EditorAddObject editorAddObject;
        private static EditorDeleteObject editorDeleteObject;
        private static EditorSetObjectProperty editorSetObjectProperty;

        internal delegate void TransactionPublishedEventHandler(EditorCommandList commands);
        internal static event TransactionPublishedEventHandler TransactionPublished;

        private static ref EditorMemory GetEditorMemory()
        {
            lock (reloadLock)
            {
                Span<EditorMemory> memorySpan;
                unsafe
                {
                    void* ptr = appMemoryDataPtr.ToPointer();
                    int size = Marshal.SizeOf<EditorMemory>();
                    memorySpan = new Span<EditorMemory>(ptr, size);
                }
                return ref memorySpan[0];
            }
        }

        internal static void Initialize(IntPtr appMemoryDataPtr, int appMemorySizeInBytes,
            PlatformLogMessage logMessage, PlatformGetFileLastWriteTime getFileLastWriteTime,
            PlatformGetFileSize getFileSize, PlatformReadEntireFile readEntireFile,
            PlatformWriteEntireFile writeEntireFile,
            PlatformStartPerfCounter startPerfCounter, PlatformEndPerfCounter endPerfCounter,
            Func<string, IntPtr> loadLibrary, Func<IntPtr, string, IntPtr> getProcAddress,
            Func<IntPtr, bool> freeLibrary)
        {
            EditorCore.loadLibrary = loadLibrary;
            EditorCore.getProcAddress = getProcAddress;
            EditorCore.freeLibrary = freeLibrary;

            EditorCore.appMemoryDataPtr = appMemoryDataPtr;

            ref EditorMemory memory = ref GetEditorMemory();
            memory.Data.BaseAddress = appMemoryDataPtr + Marshal.SizeOf<EditorMemory>();
            memory.Data.Size = (ulong)(appMemorySizeInBytes - Marshal.SizeOf<EditorMemory>());
            memory.Data.Used = 0;
            memory.PlatformApi.LogMessage = Marshal.GetFunctionPointerForDelegate(logMessage);
            memory.PlatformApi.GetFileLastWriteTime = Marshal.GetFunctionPointerForDelegate(getFileLastWriteTime);
            memory.PlatformApi.GetFileSize = Marshal.GetFunctionPointerForDelegate(getFileSize);
            memory.PlatformApi.ReadEntireFile = Marshal.GetFunctionPointerForDelegate(readEntireFile);
            memory.PlatformApi.WriteEntireFile = Marshal.GetFunctionPointerForDelegate(writeEntireFile);
            memory.PlatformApi.NotifyAssetRegistered = Marshal.GetFunctionPointerForDelegate(onAssetRegistered);
            memory.PlatformApi.PublishTransaction = Marshal.GetFunctionPointerForDelegate(onTransactionPublished);
            memory.PlatformApi.StartPerfCounter = Marshal.GetFunctionPointerForDelegate(startPerfCounter);
            memory.PlatformApi.EndPerfCounter = Marshal.GetFunctionPointerForDelegate(endPerfCounter);
        }

        internal static bool ReloadCode(string dllPath, string dllShadowCopyPath)
        {
            lock (reloadLock)
            {
                if (moduleHandle != IntPtr.Zero)
                {
                    freeLibrary(moduleHandle);
                    moduleHandle = IntPtr.Zero;
                }

                bool didShadowCopySucceed = false;
                try
                {
                    File.Copy(dllPath, dllShadowCopyPath, true);
                    didShadowCopySucceed = true;
                }
                catch
                {
                    // ignore - we will return false and retry the load again the next tick
                }

                if (didShadowCopySucceed)
                {
                    moduleHandle = loadLibrary(dllShadowCopyPath);
                }

                T GetApi<T>(string procName) where T : Delegate
                {
                    if (moduleHandle == IntPtr.Zero) return null;

                    IntPtr functionPtr = getProcAddress(moduleHandle, procName);
                    return functionPtr == IntPtr.Zero
                        ? null
                        : Marshal.GetDelegateForFunctionPointer<T>(functionPtr);
                }

                editorUpdate = GetApi<EditorUpdate>("editorUpdate");
                editorRenderSceneView = GetApi<EditorRenderSceneView>("editorRenderSceneView");
                editorRenderHeightmapPreview = GetApi<EditorRenderHeightmapPreview>("editorRenderHeightmapPreview");
                editorGetImportedHeightmapAssetHandle = GetApi<EditorGetImportedHeightmapAssetHandle>("editorGetImportedHeightmapAssetHandle");
                editorSaveHeightmap = GetApi<EditorSaveHeightmap>("editorSaveHeightmap");
                editorGetUiState = GetApi<EditorGetUiState>("editorGetUiState");
                editorAddMaterial = GetApi<EditorAddMaterial>("editorAddMaterial");
                editorDeleteMaterial = GetApi<EditorDeleteMaterial>("editorDeleteMaterial");
                editorSwapMaterial = GetApi<EditorSwapMaterial>("editorSwapMaterial");
                editorSetMaterialTexture = GetApi<EditorSetMaterialTexture>("editorSetMaterialTexture");
                editorSetMaterialProperties = GetApi<EditorSetMaterialProperties>("editorSetMaterialProperties");
                editorGetObjectProperty = GetApi<EditorGetObjectProperty>("editorGetObjectProperty");
                editorBeginTransaction = GetApi<EditorBeginTransaction>("editorBeginTransaction");
                editorClearTransaction = GetApi<EditorClearTransaction>("editorClearTransaction");
                editorCommitTransaction = GetApi<EditorCommitTransaction>("editorCommitTransaction");
                editorDiscardTransaction = GetApi<EditorDiscardTransaction>("editorDiscardTransaction");
                editorAddObject = GetApi<EditorAddObject>("editorAddObject");
                editorDeleteObject = GetApi<EditorDeleteObject>("editorDeleteObject");
                editorSetObjectProperty = GetApi<EditorSetObjectProperty>("editorSetObjectProperty");

                return moduleHandle != IntPtr.Zero;
            }
        }

        private static void OnTransactionPublished(ref byte commandBufferBaseAddress)
        {
            // the first 8 bytes contain the size of the command buffer
            ReadOnlySpan<byte> bufferSizeSpan = MemoryMarshal.CreateReadOnlySpan<byte>(
                ref commandBufferBaseAddress, sizeof(ulong));
            ref readonly ulong bufferSize = ref MemoryMarshal.AsRef<ulong>(bufferSizeSpan);

            ReadOnlySpan<byte> byteSpan = MemoryMarshal.CreateReadOnlySpan(
                ref commandBufferBaseAddress, (int)bufferSize);
            ReadOnlySpan<byte> commandsSpan = byteSpan.Slice(sizeof(ulong));
            EditorCommandList commands = new EditorCommandList(commandsSpan);
            TransactionPublished?.Invoke(commands);
        }

        private static void OnAssetRegistered(in AssetRegistration assetReg)
        {
            AssetRegistered?.Invoke(in assetReg);
        }

        internal static void Update(float deltaTime)
            => editorUpdate?.Invoke(ref GetEditorMemory(), deltaTime);

        internal static void RenderSceneView(ref EditorViewContext vctx, float deltaTime, ref EditorInput input)
            => editorRenderSceneView?.Invoke(ref GetEditorMemory(), ref vctx, deltaTime, ref input);

        internal static void RenderHeightmapPreview(ref EditorViewContext vctx, float deltaTime, ref EditorInput input)
            => editorRenderHeightmapPreview?.Invoke(ref GetEditorMemory(), ref vctx, deltaTime, ref input);

        internal static IntPtr GetImportedHeightmapAssetHandle()
            => editorGetImportedHeightmapAssetHandle?.Invoke(ref GetEditorMemory()) ?? IntPtr.Zero;

        internal static void SaveHeightmap(string filePath)
            => editorSaveHeightmap?.Invoke(ref GetEditorMemory(), filePath);

        internal static ref EditorUiState GetUiState()
        {
            if (editorGetUiState == null)
            {
                return ref defaultEditorState;
            }
            else
            {
                ref EditorMemory memory = ref GetEditorMemory();
                return ref editorGetUiState(ref memory);
            }
        }

        internal static void AddMaterial(TerrainMaterialProperties props)
            => editorAddMaterial?.Invoke(ref GetEditorMemory(), props);

        internal static void DeleteMaterial(int index)
            => editorDeleteMaterial?.Invoke(ref GetEditorMemory(), (uint)index);

        internal static void SwapMaterial(int indexA, int indexB)
            => editorSwapMaterial?.Invoke(ref GetEditorMemory(), (uint)indexA, (uint)indexB);

        internal static void SetMaterialTexture(uint materialId,
            TerrainMaterialTextureType textureType, IntPtr assetHandle)
            => editorSetMaterialTexture?.Invoke(ref GetEditorMemory(), materialId, textureType, assetHandle);

        internal static void SetMaterialProperties(uint materialId, float textureSize,
            float slopeStart, float slopeEnd, float altitudeStart, float altitudeEnd)
        {
            editorSetMaterialProperties?.Invoke(ref GetEditorMemory(), materialId, textureSize,
                slopeStart, slopeEnd, altitudeStart, altitudeEnd);
        }

        internal static float GetObjectProperty(uint objectId, ObjectProperty property)
            => editorGetObjectProperty?.Invoke(ref GetEditorMemory(), objectId, property) ?? 0;

        internal static bool BeginTransaction(out Transaction tx)
        {
            tx = new Transaction(editorBeginTransaction?.Invoke(ref GetEditorMemory()) ?? IntPtr.Zero);
            return tx.Pointer != IntPtr.Zero;
        }

        internal static void ClearTransaction(Transaction tx)
            => editorClearTransaction?.Invoke(tx.Pointer);

        internal static void CommitTransaction(Transaction tx)
            => editorCommitTransaction?.Invoke(tx.Pointer);

        internal static void DiscardTransaction(Transaction tx)
            => editorDiscardTransaction?.Invoke(tx.Pointer);

        internal static void AddObject(Transaction tx)
            => editorAddObject?.Invoke(ref GetEditorMemory(), tx.Pointer);

        internal static void DeleteObject(Transaction tx, uint objectId)
            => editorDeleteObject?.Invoke(tx.Pointer, objectId);

        internal static void SetObjectProperty(
            Transaction tx, uint objectId, ObjectProperty property, float value)
            => editorSetObjectProperty?.Invoke(tx.Pointer, objectId, property, value);
    }

    internal static class EditorExtensions
    {
        internal static AssetFileState GetFileState(this AssetRegistration assetReg)
        {
            return Marshal.PtrToStructure<AssetFileState>(assetReg.StatePtr);
        }
    }
}