using System;
using System.IO;
using System.Runtime.InteropServices;
using Terrain.Editor.Engine;

namespace Terrain.Editor.Core
{
    internal delegate void PlatformCaptureMouse();
    internal delegate void PlatformPublishTransaction(
        ref byte commandBufferBaseAddress, ulong commandBufferSize);

    [StructLayout(LayoutKind.Sequential)]
    internal struct EditorMemory
    {
        public MemoryBlock Data;
        public ulong DataStorageUsed;

        public PlatformCaptureMouse PlatformCaptureMouse;
        public PlatformPublishTransaction PlatformPublishTransaction;

        public IntPtr EngineApiPtr;
        public IntPtr EngineMemoryPtr;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct EditorInput
    {
        public IntPtr ActiveViewState;
        public float ScrollOffset;
        public Vector2 NormalizedCursorPos;
        public Vector2 CursorOffset;
        public ulong PressedButtons;
        public ulong PreviousPressedButtons;
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
        KeyRightShift = 1L << 60,
        KeyRightControl = 1L << 61,
        KeyAlt = 1L << 62
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct EditorViewContext
    {
        public IntPtr ViewState;
        public uint X;
        public uint Y;
        public uint Width;
        public uint Height;
    }

    public enum TerrainBrushTool
    {
        Raise = 0,
        Lower = 1,
        Flatten = 2,
        Smooth = 3
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct EditorUiState
    {
        public TerrainBrushTool TerrainBrushTool;
        public float TerrainBrushRadius;
        public float TerrainBrushFalloff;
        public float TerrainBrushStrength;

        public float SceneLightDirection;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct TerrainMaterialProperties
    {
        public uint AlbedoTextureAssetId;
        public uint NormalTextureAssetId;
        public uint DisplacementTextureAssetId;
        public uint AoTextureAssetId;
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
        SetObjectTransform
    }

    [StructLayout(LayoutKind.Sequential)]
    struct AddMaterialCommand
    {
        public uint MaterialId;

        public uint AlbedoTextureAssetId;
        public uint NormalTextureAssetId;
        public uint DisplacementTextureAssetId;
        public uint AoTextureAssetId;
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
        public uint AssetId;
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
    struct SetObjectTransformCommand
    {
        public uint ObjectId;
        public Vector3 Position;
        public Vector3 Rotation;
        public Vector3 Scale;
    }

    internal static class EditorCore
    {
        private static EditorMemory memory;
        private static IntPtr moduleHandle;

        private static Func<string, IntPtr> loadLibrary;
        private static Func<IntPtr, string, IntPtr> getProcAddress;
        private static Func<IntPtr, bool> freeLibrary;

        private static EditorUiState defaultEditorState = default(EditorUiState);

        delegate void EditorUpdate(ref EditorMemory memory, float deltaTime, ref EditorInput input);
        delegate void EditorRenderSceneView(ref EditorMemory memory, ref EditorViewContext view);
        delegate void EditorRenderHeightmapPreview(ref EditorMemory memory, ref EditorViewContext view);
        delegate uint EditorGetImportedHeightmapAssetId(ref EditorMemory memory);
        delegate ref EditorUiState EditorGetUiState(ref EditorMemory memory);
        delegate void EditorAddMaterial(ref EditorMemory memory, TerrainMaterialProperties props);
        delegate void EditorDeleteMaterial(ref EditorMemory memory, uint index);
        delegate void EditorSwapMaterial(ref EditorMemory memory, uint indexA, uint indexB);
        delegate void EditorSetMaterialTexture(ref EditorMemory memory, uint materialId,
            TerrainMaterialTextureType textureType, uint assetId);
        delegate void EditorSetMaterialProperties(ref EditorMemory memory, uint materialId, float textureSize,
            float slopeStart, float slopeEnd, float altitudeStart, float altitudeEnd);
        delegate void EditorSetObjectTransform(ref EditorMemory memory, uint objectId,
            float positionX, float positionY, float positionZ,
            float rotationX, float rotationY, float rotationZ,
            float scaleX, float scaleY, float scaleZ);

        private static EditorUpdate editorUpdate;
        private static EditorRenderSceneView editorRenderSceneView;
        private static EditorRenderHeightmapPreview editorRenderHeightmapPreview;
        private static EditorGetImportedHeightmapAssetId editorGetImportedHeightmapAssetId;
        private static EditorGetUiState editorGetUiState;
        private static EditorAddMaterial editorAddMaterial;
        private static EditorDeleteMaterial editorDeleteMaterial;
        private static EditorSwapMaterial editorSwapMaterial;
        private static EditorSetMaterialTexture editorSetMaterialTexture;
        private static EditorSetMaterialProperties editorSetMaterialProperties;
        private static EditorSetObjectTransform editorSetObjectTransform;

        internal delegate void TransactionPublishedEventHandler(EditorCommandList commands);
        internal static event TransactionPublishedEventHandler TransactionPublished;

        internal static void Initialize(IntPtr editorMemoryDataPtr, int editorMemorySizeInBytes,
            PlatformCaptureMouse captureMouse, IntPtr engineMemoryPtr,
            Func<string, IntPtr> loadLibrary, Func<IntPtr, string, IntPtr> getProcAddress,
            Func<IntPtr, bool> freeLibrary)
        {
            EditorCore.loadLibrary = loadLibrary;
            EditorCore.getProcAddress = getProcAddress;
            EditorCore.freeLibrary = freeLibrary;

            memory = new EditorMemory
            {
                Data = new MemoryBlock
                {
                    BaseAddress = editorMemoryDataPtr,
                    Size = (ulong)editorMemorySizeInBytes
                },
                DataStorageUsed = 0,
                PlatformCaptureMouse = captureMouse,
                PlatformPublishTransaction = OnTransactionPublished,
                EngineMemoryPtr = engineMemoryPtr
            };
        }

        internal static void UpdateEngineApi(IntPtr engineApiPtr)
        {
            memory.EngineApiPtr = engineApiPtr;
        }

        internal static bool ReloadCode(string dllPath, string dllShadowCopyPath)
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
            editorGetImportedHeightmapAssetId = GetApi<EditorGetImportedHeightmapAssetId>("editorGetImportedHeightmapAssetId");
            editorGetUiState = GetApi<EditorGetUiState>("editorGetUiState");
            editorAddMaterial = GetApi<EditorAddMaterial>("editorAddMaterial");
            editorDeleteMaterial = GetApi<EditorDeleteMaterial>("editorDeleteMaterial");
            editorSwapMaterial = GetApi<EditorSwapMaterial>("editorSwapMaterial");
            editorSetMaterialTexture = GetApi<EditorSetMaterialTexture>("editorSetMaterialTexture");
            editorSetMaterialProperties = GetApi<EditorSetMaterialProperties>("editorSetMaterialProperties");
            editorSetObjectTransform = GetApi<EditorSetObjectTransform>("editorSetObjectTransform");

            return moduleHandle != IntPtr.Zero;
        }

        private static void OnTransactionPublished(
            ref byte commandBufferBaseAddress, ulong commandBufferSize)
        {
            ReadOnlySpan<byte> byteSpan = MemoryMarshal.CreateReadOnlySpan(
                ref commandBufferBaseAddress, (int)commandBufferSize);
            EditorCommandList commands = new EditorCommandList(byteSpan);
            TransactionPublished?.Invoke(commands);
        }

        internal static void Update(float deltaTime, ref EditorInput input)
            => editorUpdate?.Invoke(ref memory, deltaTime, ref input);

        internal static void RenderSceneView(ref EditorViewContext vctx)
            => editorRenderSceneView?.Invoke(ref memory, ref vctx);

        internal static void RenderHeightmapPreview(ref EditorViewContext vctx)
            => editorRenderHeightmapPreview?.Invoke(ref memory, ref vctx);

        internal static uint? GetImportedHeightmapAssetId()
            => editorGetImportedHeightmapAssetId?.Invoke(ref memory);

        internal static ref EditorUiState GetUiState()
        {
            if (editorGetUiState == null)
            {
                return ref defaultEditorState;
            }
            else
            {
                return ref editorGetUiState(ref memory);
            }
        }

        internal static void AddMaterial(TerrainMaterialProperties props)
            => editorAddMaterial?.Invoke(ref memory, props);

        internal static void DeleteMaterial(int index)
            => editorDeleteMaterial?.Invoke(ref memory, (uint)index);

        internal static void SwapMaterial(int indexA, int indexB)
            => editorSwapMaterial?.Invoke(ref memory, (uint)indexA, (uint)indexB);

        internal static void SetMaterialTexture(uint materialId,
            TerrainMaterialTextureType textureType, uint assetId)
            => editorSetMaterialTexture?.Invoke(ref memory, materialId, textureType, assetId);

        internal static void SetMaterialProperties(uint materialId, float textureSize,
            float slopeStart, float slopeEnd, float altitudeStart, float altitudeEnd)
        {
            editorSetMaterialProperties?.Invoke(ref memory, materialId, textureSize,
                slopeStart, slopeEnd, altitudeStart, altitudeEnd);
        }

        internal static void SetObjectTransform(
            uint objectId,
            float positionX, float positionY, float positionZ,
            float rotationX, float rotationY, float rotationZ,
            float scaleX, float scaleY, float scaleZ)
        {
            editorSetObjectTransform?.Invoke(ref memory, objectId,
                positionX, positionY, positionZ,
                rotationX, rotationY, rotationZ,
                scaleX, scaleY, scaleZ);
        }
    }
}
