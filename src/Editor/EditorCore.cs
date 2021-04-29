using System;
using System.Runtime.InteropServices;
using Terrain.Engine.Interop;

namespace Terrain.Editor
{
    using EditorInput = IntPtr;
    using EditorViewContext = IntPtr;
    using TerrainBrushParameters = IntPtr;
    using MaterialProperties = IntPtr;

    internal enum EditorTool
    {
        RaiseTerrain = 0,
        LowerTerrain = 1,
        FlattenTerrain = 2,
        SmoothTerrain = 3
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
        private static IntPtr memoryPtr;
        private static IntPtr moduleHandle;

        delegate void EditorUpdate(IntPtr memory, float deltaTime, EditorInput input);
        delegate void EditorShutdown(IntPtr memory);
        delegate void EditorRenderSceneView(IntPtr memory, EditorViewContext view);
        delegate void EditorRenderHeightmapPreview(IntPtr memory, EditorViewContext view);
        delegate uint EditorGetImportedHeightmapAssetId(IntPtr memory);
        delegate EditorTool EditorGetBrushTool(IntPtr memory);
        delegate void EditorSetBrushTool(IntPtr memory, EditorTool tool);
        delegate TerrainBrushParameters EditorGetBrushParameters(IntPtr memory);
        delegate void EditorSetBrushParameters(IntPtr memory, float radius, float falloff, float strength);
        delegate void EditorAddMaterial(IntPtr memory, MaterialProperties props);
        delegate void EditorDeleteMaterial(IntPtr memory, uint index);
        delegate void EditorSwapMaterial(IntPtr memory, uint indexA, uint indexB);
        delegate MaterialProperties EditorGetMaterialProperties(IntPtr memory, uint index);
        delegate void EditorSetMaterialTexture(IntPtr memory, uint index,
            TerrainMaterialTextureType textureType, uint assetId);
        delegate void EditorSetMaterialProperties(IntPtr memory, uint index, float textureSize,
            float slopeStart, float slopeEnd, float altitudeStart, float altitudeEnd);
        delegate void EditorSetRockTransform(IntPtr memory, float positionX, float positionY,
            float positionZ, float rotationX, float rotationY, float rotationZ, float scaleX, float scaleY, float scaleZ);
        delegate void EditorSetSceneParameters(IntPtr memory, float lightDirection);

        private static EditorUpdate editorUpdate;
        private static EditorShutdown editorShutdown;
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

        internal static void Initialize(IntPtr editorMemoryPtr)
        {
            memoryPtr = editorMemoryPtr;
        }

        internal static void ReloadCode(string dllPath, string dllShadowCopyPath)
        {
            moduleHandle = EngineInterop.ReloadEditorCode(dllPath, dllShadowCopyPath);

            T GetApi<T>(string procName) where T : Delegate
            {
                IntPtr functionPtr = Win32.GetProcAddress(moduleHandle, procName);
                return functionPtr == IntPtr.Zero
                    ? null
                    : Marshal.GetDelegateForFunctionPointer<T>(functionPtr);
            }

            editorUpdate = GetApi<EditorUpdate>("editorUpdate");
            editorShutdown = GetApi<EditorShutdown>("editorShutdown");
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
        }

        internal static void Update(float deltaTime, EditorInputProxy input)
            => EngineInterop.Update(deltaTime, input);

        internal static void RenderSceneView(ref EditorViewContextProxy vctx)
            => EngineInterop.RenderSceneView(ref vctx);

        internal static void RenderHeightmapPreview(ref EditorViewContextProxy vctx)
            => EngineInterop.RenderHeightmapPreview(ref vctx);

        internal static void Shutdown()
            => editorShutdown?.Invoke(memoryPtr);

        internal static void LoadHeightmapTexture(string path)
        {
            uint importedHeightmapAssetId =
                editorGetImportedHeightmapAssetId?.Invoke(memoryPtr) ?? 0;
            if (importedHeightmapAssetId > 0)
            {
                EditorPlatform.QueueAssetLoad(importedHeightmapAssetId, path);
            }
        }

        internal static EditorTool GetBrushTool()
            => editorGetBrushTool?.Invoke(memoryPtr) ?? EditorTool.RaiseTerrain;

        internal static void SetBrushTool(EditorTool tool)
            => editorSetBrushTool?.Invoke(memoryPtr, tool);

        internal static TerrainBrushParametersProxy GetBrushParameters()
            => EngineInterop.GetBrushParameters();

        internal static void SetBrushParameters(float radius, float falloff, float strength)
            => editorSetBrushParameters?.Invoke(memoryPtr, radius, falloff, strength);

        internal static void AddMaterial(MaterialProps props)
            => EngineInterop.AddMaterial(props);

        internal static void DeleteMaterial(int index)
            => editorDeleteMaterial?.Invoke(memoryPtr, (uint)index);

        internal static void SwapMaterial(int indexA, int indexB)
            => editorSwapMaterial?.Invoke(memoryPtr, (uint)indexA, (uint)indexB);

        internal static MaterialProps GetMaterialProperties(int index)
            => EngineInterop.GetMaterialProperties(index);

        internal static void SetMaterialTexture(int index,
            TerrainMaterialTextureType textureType, uint assetId)
            => editorSetMaterialTexture?.Invoke(memoryPtr, (uint)index, textureType, assetId);

        internal static void SetMaterialProperties(int index, float textureSize,
            float slopeStart, float slopeEnd, float altitudeStart, float altitudeEnd)
        {
            editorSetMaterialProperties?.Invoke(memoryPtr, (uint)index, textureSize,
                slopeStart, slopeEnd, altitudeStart, altitudeEnd);
        }

        internal static void SetRockTransform(
            float positionX, float positionY, float positionZ,
            float rotationX, float rotationY, float rotationZ,
            float scaleX, float scaleY, float scaleZ)
        {
            editorSetRockTransform?.Invoke(memoryPtr,
                positionX, positionY, positionZ,
                rotationX, rotationY, rotationZ,
                scaleX, scaleY, scaleZ);
        }

        internal static void SetSceneParameters(float lightDirection)
            => editorSetSceneParameters?.Invoke(memoryPtr, lightDirection);
    }
}
