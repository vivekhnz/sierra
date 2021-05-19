using System.Collections.ObjectModel;
using Terrain.Editor.Core;

namespace Terrain.Editor.ViewModels
{
    public class EditorDocumentViewModel : ViewModelBase
    {
        public ObservableCollection<TerrainMaterialViewModel> TerrainMaterials { get; private set; }
            = new ObservableCollection<TerrainMaterialViewModel>();

        internal void OnTransactionPublished(EditorCommandList commands)
        {
            foreach (var entry in commands)
            {
                if (entry.Type == EditorCommandType.AddMaterial)
                {
                    ref readonly AddMaterialCommand cmd = ref entry.As<AddMaterialCommand>();

                    var materialVm = new TerrainMaterialViewModel
                    {
                        Index = TerrainMaterials.Count,
                        Name = $"Material {TerrainMaterials.Count + 1}",
                        AlbedoTextureAssetId = cmd.AlbedoTextureAssetId,
                        NormalTextureAssetId = cmd.NormalTextureAssetId,
                        DisplacementTextureAssetId = cmd.DisplacementTextureAssetId,
                        AoTextureAssetId = cmd.AoTextureAssetId,
                        TextureSizeInWorldUnits = cmd.TextureSizeInWorldUnits,
                        SlopeStart = cmd.SlopeStart,
                        SlopeEnd = cmd.SlopeEnd,
                        AltitudeStart = cmd.AltitudeStart,
                        AltitudeEnd = cmd.AltitudeEnd
                    };
                    TerrainMaterials.Add(materialVm);
                }
                else if (entry.Type == EditorCommandType.DeleteMaterial)
                {
                    ref readonly DeleteMaterialCommand cmd = ref entry.As<DeleteMaterialCommand>();

                    TerrainMaterials.RemoveAt((int)cmd.Index);
                }
                else if (entry.Type == EditorCommandType.SwapMaterial)
                {
                    ref readonly SwapMaterialCommand cmd = ref entry.As<SwapMaterialCommand>();

                    TerrainMaterials.Move((int)cmd.IndexA, (int)cmd.IndexB);
                }
            }
            for (int i = 0; i < TerrainMaterials.Count; i++)
            {
                TerrainMaterials[i].Index = i;
            }
        }
    }
}