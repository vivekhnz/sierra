using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Terrain.Editor
{
    [StructLayout(LayoutKind.Sequential)]
    internal struct Vector2
    {
        public float X;
        public float Y;

        public Vector2(float x, float y)
        {
            X = x;
            Y = y;
        }
    }

    internal delegate void PlatformCaptureMouse();

    [StructLayout(LayoutKind.Sequential)]
    internal struct EditorMemory
    {
        public MemoryBlock Data;
        public ulong DataStorageUsed;

        public PlatformCaptureMouse PlatformCaptureMouse;

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

    [StructLayout(LayoutKind.Sequential)]
    internal struct EditorViewContext
    {
        public IntPtr ViewState;
        public uint X;
        public uint Y;
        public uint Width;
        public uint Height;
    }

    internal enum EditorTool
    {
        RaiseTerrain = 0,
        LowerTerrain = 1,
        FlattenTerrain = 2,
        SmoothTerrain = 3
    }

    [StructLayout(LayoutKind.Sequential)]
    struct TerrainBrushParameters
    {
        public float Radius;
        public float Falloff;
        public float Strength;
    };

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
    };

    internal static class EditorCore
    {
        private static EditorMemory memory;
        private static IntPtr moduleHandle;

        delegate void EditorUpdate(ref EditorMemory memory, float deltaTime, ref EditorInput input);
        delegate void EditorRenderSceneView(ref EditorMemory memory, ref EditorViewContext view);
        delegate void EditorRenderHeightmapPreview(ref EditorMemory memory, ref EditorViewContext view);
        delegate uint EditorGetImportedHeightmapAssetId(ref EditorMemory memory);
        delegate EditorTool EditorGetBrushTool(ref EditorMemory memory);
        delegate void EditorSetBrushTool(ref EditorMemory memory, EditorTool tool);
        delegate TerrainBrushParameters EditorGetBrushParameters(ref EditorMemory memory);
        delegate void EditorSetBrushParameters(ref EditorMemory memory, float radius, float falloff, float strength);
        delegate void EditorAddMaterial(ref EditorMemory memory, TerrainMaterialProperties props);
        delegate void EditorDeleteMaterial(ref EditorMemory memory, uint index);
        delegate void EditorSwapMaterial(ref EditorMemory memory, uint indexA, uint indexB);
        delegate TerrainMaterialProperties EditorGetMaterialProperties(ref EditorMemory memory, uint index);
        delegate void EditorSetMaterialTexture(ref EditorMemory memory, uint index,
            TerrainMaterialTextureType textureType, uint assetId);
        delegate void EditorSetMaterialProperties(ref EditorMemory memory, uint index, float textureSize,
            float slopeStart, float slopeEnd, float altitudeStart, float altitudeEnd);
        delegate void EditorSetRockTransform(ref EditorMemory memory, float positionX, float positionY,
            float positionZ, float rotationX, float rotationY, float rotationZ, float scaleX, float scaleY, float scaleZ);
        delegate void EditorSetSceneParameters(ref EditorMemory memory, float lightDirection);

        private static EditorUpdate editorUpdate;
        private static EditorRenderSceneView editorRenderSceneView;
        private static EditorRenderHeightmapPreview editorRenderHeightmapPreview;
        private static EditorGetImportedHeightmapAssetId editorGetImportedHeightmapAssetId;
        private static EditorGetBrushTool editorGetBrushTool;
        private static EditorSetBrushTool editorSetBrushTool;
        private static EditorGetBrushParameters editorGetBrushParameters;
        private static EditorSetBrushParameters editorSetBrushParameters;
        private static EditorAddMaterial editorAddMaterial;
        private static EditorDeleteMaterial editorDeleteMaterial;
        private static EditorSwapMaterial editorSwapMaterial;
        private static EditorGetMaterialProperties editorGetMaterialProperties;
        private static EditorSetMaterialTexture editorSetMaterialTexture;
        private static EditorSetMaterialProperties editorSetMaterialProperties;
        private static EditorSetRockTransform editorSetRockTransform;
        private static EditorSetSceneParameters editorSetSceneParameters;

        internal static void Initialize(IntPtr editorMemoryDataPtr, int editorMemorySizeInBytes,
            PlatformCaptureMouse captureMouse, IntPtr engineMemoryPtr)
        {
            memory = new EditorMemory
            {
                Data = new MemoryBlock
                {
                    BaseAddress = editorMemoryDataPtr,
                    Size = (ulong)editorMemorySizeInBytes
                },
                DataStorageUsed = 0,
                PlatformCaptureMouse = captureMouse,
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
                Win32.FreeLibrary(moduleHandle);
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
                moduleHandle = Win32.LoadLibrary(dllShadowCopyPath);
            }

            T GetApi<T>(string procName) where T : Delegate
            {
                if (moduleHandle == IntPtr.Zero) return null;

                IntPtr functionPtr = Win32.GetProcAddress(moduleHandle, procName);
                return functionPtr == IntPtr.Zero
                    ? null
                    : Marshal.GetDelegateForFunctionPointer<T>(functionPtr);
            }

            editorUpdate = GetApi<EditorUpdate>("editorUpdate");
            editorRenderSceneView = GetApi<EditorRenderSceneView>("editorRenderSceneView");
            editorRenderHeightmapPreview = GetApi<EditorRenderHeightmapPreview>("editorRenderHeightmapPreview");
            editorGetImportedHeightmapAssetId = GetApi<EditorGetImportedHeightmapAssetId>("editorGetImportedHeightmapAssetId");
            editorGetBrushTool = GetApi<EditorGetBrushTool>("editorGetBrushTool");
            editorSetBrushTool = GetApi<EditorSetBrushTool>("editorSetBrushTool");
            editorGetBrushParameters = GetApi<EditorGetBrushParameters>("editorGetBrushParameters");
            editorSetBrushParameters = GetApi<EditorSetBrushParameters>("editorSetBrushParameters");
            editorAddMaterial = GetApi<EditorAddMaterial>("editorAddMaterial");
            editorDeleteMaterial = GetApi<EditorDeleteMaterial>("editorDeleteMaterial");
            editorSwapMaterial = GetApi<EditorSwapMaterial>("editorSwapMaterial");
            editorGetMaterialProperties = GetApi<EditorGetMaterialProperties>("editorGetMaterialProperties");
            editorSetMaterialTexture = GetApi<EditorSetMaterialTexture>("editorSetMaterialTexture");
            editorSetMaterialProperties = GetApi<EditorSetMaterialProperties>("editorSetMaterialProperties");
            editorSetRockTransform = GetApi<EditorSetRockTransform>("editorSetRockTransform");
            editorSetSceneParameters = GetApi<EditorSetSceneParameters>("editorSetSceneParameters");

            return moduleHandle != IntPtr.Zero;
        }

        internal static void Update(float deltaTime, ref EditorInput input)
            => editorUpdate?.Invoke(ref memory, deltaTime, ref input);

        internal static void RenderSceneView(ref EditorViewContext vctx)
            => editorRenderSceneView?.Invoke(ref memory, ref vctx);

        internal static void RenderHeightmapPreview(ref EditorViewContext vctx)
            => editorRenderHeightmapPreview?.Invoke(ref memory, ref vctx);

        internal static void LoadHeightmapTexture(string path)
        {
            uint importedHeightmapAssetId =
                editorGetImportedHeightmapAssetId?.Invoke(ref memory) ?? 0;
            if (importedHeightmapAssetId > 0)
            {
                EditorPlatform.QueueAssetLoad(importedHeightmapAssetId, path);
            }
        }

        internal static EditorTool GetBrushTool()
            => editorGetBrushTool?.Invoke(ref memory) ?? EditorTool.RaiseTerrain;

        internal static void SetBrushTool(EditorTool tool)
            => editorSetBrushTool?.Invoke(ref memory, tool);

        internal static TerrainBrushParameters GetBrushParameters()
            => editorGetBrushParameters?.Invoke(ref memory) ?? default(TerrainBrushParameters);

        internal static void SetBrushParameters(float radius, float falloff, float strength)
            => editorSetBrushParameters?.Invoke(ref memory, radius, falloff, strength);

        internal static void AddMaterial(TerrainMaterialProperties props)
            => editorAddMaterial?.Invoke(ref memory, props);

        internal static void DeleteMaterial(int index)
            => editorDeleteMaterial?.Invoke(ref memory, (uint)index);

        internal static void SwapMaterial(int indexA, int indexB)
            => editorSwapMaterial?.Invoke(ref memory, (uint)indexA, (uint)indexB);

        internal static TerrainMaterialProperties GetMaterialProperties(int index)
            => editorGetMaterialProperties?.Invoke(ref memory, (uint)index)
                ?? default(TerrainMaterialProperties);

        internal static void SetMaterialTexture(int index,
            TerrainMaterialTextureType textureType, uint assetId)
            => editorSetMaterialTexture?.Invoke(ref memory, (uint)index, textureType, assetId);

        internal static void SetMaterialProperties(int index, float textureSize,
            float slopeStart, float slopeEnd, float altitudeStart, float altitudeEnd)
        {
            editorSetMaterialProperties?.Invoke(ref memory, (uint)index, textureSize,
                slopeStart, slopeEnd, altitudeStart, altitudeEnd);
        }

        internal static void SetRockTransform(
            float positionX, float positionY, float positionZ,
            float rotationX, float rotationY, float rotationZ,
            float scaleX, float scaleY, float scaleZ)
        {
            editorSetRockTransform?.Invoke(ref memory,
                positionX, positionY, positionZ,
                rotationX, rotationY, rotationZ,
                scaleX, scaleY, scaleZ);
        }

        internal static void SetSceneParameters(float lightDirection)
            => editorSetSceneParameters?.Invoke(ref memory, lightDirection);
    }
}
