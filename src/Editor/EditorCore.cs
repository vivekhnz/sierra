using Terrain.Engine.Interop;
using Terrain.Engine.Interop.Proxy;

namespace Terrain.Editor
{
    struct TerrainBrushParameters
    {
        public float Radius;
        public float Falloff;
        public float Strength;
    }

    enum TerrainMaterialTextureType
    {
        Albedo = 0,
        Normal = 1,
        Displacement = 2,
        AmbientOcclusion = 3
    }

    internal static class EditorCore
    {
        internal static void RenderSceneView(ref EditorViewContextProxy vctx)
        {
            EngineInterop.RenderSceneView(ref vctx);
        }

        internal static void RenderHeightmapPreview(ref EditorViewContextProxy vctx)
        {
            EngineInterop.RenderHeightmapPreview(ref vctx);
        }

        internal static void LoadHeightmapTexture(string path)
        {
            EngineInterop.LoadHeightmapTexture(path);
        }

        internal static EditorToolProxy GetBrushTool()
            => EngineInterop.State.CurrentTool;

        internal static void SetBrushTool(EditorToolProxy tool)
        {
            EngineInterop.State.CurrentTool = tool;
        }

        internal static TerrainBrushParameters GetBrushParameters()
        {
            return new TerrainBrushParameters
            {
                Radius = EngineInterop.State.BrushRadius,
                Falloff = EngineInterop.State.BrushFalloff,
                Strength = EngineInterop.State.BrushStrength
            };
        }
        internal static void SetBrushParameters(float radius, float falloff, float strength)
        {
            EngineInterop.State.BrushRadius = radius;
            EngineInterop.State.BrushFalloff = falloff;
            EngineInterop.State.BrushStrength = strength;
        }

        internal static void AddMaterial(MaterialProps props)
        {
            EngineInterop.AddMaterial(props);
        }

        internal static void DeleteMaterial(int index)
        {
            EngineInterop.DeleteMaterial(index);
        }

        internal static void SwapMaterial(int indexA, int indexB)
        {
            EngineInterop.SwapMaterial(indexA, indexB);
        }

        internal static MaterialProps GetMaterialProperties(int index)
            => EngineInterop.GetMaterialProperties(index);

        internal static void SetMaterialTexture(int index, TerrainMaterialTextureType textureType, uint assetId)
        {
            switch (textureType)
            {
                case TerrainMaterialTextureType.Albedo:
                    EngineInterop.SetMaterialAlbedoTexture(index, assetId);
                    break;
                case TerrainMaterialTextureType.Normal:
                    EngineInterop.SetMaterialNormalTexture(index, assetId);
                    break;
                case TerrainMaterialTextureType.Displacement:
                    EngineInterop.SetMaterialDisplacementTexture(index, assetId);
                    break;
                case TerrainMaterialTextureType.AmbientOcclusion:
                    EngineInterop.SetMaterialAoTexture(index, assetId);
                    break;
            }
        }

        internal static void SetMaterialParameters(int index, float textureSize, float slopeStart,
            float slopeEnd, float altitudeStart, float altitudeEnd)
        {
            EngineInterop.SetMaterialTextureSize(index, textureSize);
            EngineInterop.SetMaterialSlopeStart(index, slopeStart);
            EngineInterop.SetMaterialSlopeEnd(index, slopeEnd);
            EngineInterop.SetMaterialAltitudeStart(index, altitudeStart);
            EngineInterop.SetMaterialAltitudeEnd(index, altitudeEnd);
        }

        internal static void SetRockTransform(float positionX, float positionY, float positionZ, float rotationX, float rotationY, float rotationZ, float scaleX, float scaleY, float scaleZ)
        {
            EngineInterop.SetRockTransform(
                positionX, positionY, positionZ,
                rotationX, rotationY, rotationZ,
                scaleX, scaleY, scaleZ);
        }

        internal static void SetSceneParameters(float lightDirection)
        {
            EngineInterop.State.LightDirection = lightDirection;
        }
    }
}
