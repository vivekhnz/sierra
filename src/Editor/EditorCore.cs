using System;
using Terrain.Engine.Interop;

namespace Terrain.Editor
{
    internal static class EditorCore
    {
        internal static void Update(float deltaTime, EditorInputProxy input)
            => EngineInterop.Update(deltaTime, input);

        internal static void RenderSceneView(ref EditorViewContextProxy vctx)
            => EngineInterop.RenderSceneView(ref vctx);

        internal static void RenderHeightmapPreview(ref EditorViewContextProxy vctx)
            => EngineInterop.RenderHeightmapPreview(ref vctx);

        internal static void Shutdown()
            => EngineInterop.Shutdown();

        internal static void LoadHeightmapTexture(string path)
        {
            uint importedHeightmapAssetId = EngineInterop.GetImportedHeightmapAssetId();
            if (importedHeightmapAssetId > 0)
            {
                EditorPlatform.QueueAssetLoad(importedHeightmapAssetId, path);
            }
        }

        internal static EditorToolProxy GetBrushTool()
            => EngineInterop.GetBrushTool();

        internal static void SetBrushTool(EditorToolProxy tool)
            => EngineInterop.SetBrushTool(tool);

        internal static TerrainBrushParametersProxy GetBrushParameters()
            => EngineInterop.GetBrushParameters();

        internal static void SetBrushParameters(float radius, float falloff, float strength)
            => EngineInterop.SetBrushParameters(radius, falloff, strength);

        internal static void AddMaterial(MaterialProps props)
            => EngineInterop.AddMaterial(props);

        internal static void DeleteMaterial(int index)
            => EngineInterop.DeleteMaterial(index);

        internal static void SwapMaterial(int indexA, int indexB)
            => EngineInterop.SwapMaterial(indexA, indexB);

        internal static MaterialProps GetMaterialProperties(int index)
            => EngineInterop.GetMaterialProperties(index);

        internal static void SetMaterialTexture(int index,
            TerrainMaterialTextureTypeProxy textureType, uint assetId)
            => EngineInterop.SetMaterialTexture(index, textureType, assetId);

        internal static void SetMaterialProperties(int index, float textureSize, float slopeStart,
            float slopeEnd, float altitudeStart, float altitudeEnd)
            => EngineInterop.SetMaterialProperties(index, textureSize, slopeStart, slopeEnd,
                altitudeStart, altitudeEnd);

        internal static void SetRockTransform(
            float positionX, float positionY, float positionZ,
            float rotationX, float rotationY, float rotationZ,
            float scaleX, float scaleY, float scaleZ)
        {
            EngineInterop.SetRockTransform(
                positionX, positionY, positionZ,
                rotationX, rotationY, rotationZ,
                scaleX, scaleY, scaleZ);
        }

        internal static void SetSceneParameters(float lightDirection)
            => EngineInterop.SetSceneParameters(lightDirection);
    }
}
