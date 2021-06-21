using System.Collections.ObjectModel;
using System.Linq;
using Terrain.Editor.Core;
using Terrain.Editor.Utilities;

namespace Terrain.Editor.ViewModels
{
    public class EditorDocumentViewModel : ViewModelBase
    {
        const int maxMaterialCount = 8;

        public ObservableCollection<TerrainMaterialViewModel> TerrainMaterials { get; private set; }
            = new ObservableCollection<TerrainMaterialViewModel>();
        public ObservableCollection<uint> ObjectIds { get; private set; }
            = new ObservableCollection<uint>();

        public DelegateCommand AddMaterialCommand { get; private set; }

        public EditorDocumentViewModel()
        {
            AddMaterialCommand = new DelegateCommand(
                () =>
                {
                    uint GetTextureAssetId(string relativePath)
                    {
                        var assetVm = App.Current.Assets.RegisteredAssets.FirstOrDefault(
                            asset => asset.FileRelativePath == relativePath);
                        return assetVm?.AssetId ?? 0;
                    }

                    EditorCore.AddMaterial(new TerrainMaterialProperties
                    {
                        AlbedoTextureAssetId = GetTextureAssetId("ground_albedo.bmp"),
                        NormalTextureAssetId = GetTextureAssetId("ground_normal.bmp"),
                        DisplacementTextureAssetId = GetTextureAssetId("ground_displacement.tga"),
                        AoTextureAssetId = GetTextureAssetId("ground_ao.tga"),
                        TextureSizeInWorldUnits = 2.5f,
                        SlopeStart = 0.0f,
                        SlopeEnd = 0.0f,
                        AltitudeStart = 0.0f,
                        AltitudeEnd = 0.0f
                    });
                },
                () => TerrainMaterials.Count < maxMaterialCount);
        }

        internal void OnTransactionPublished(EditorCommandList commands)
        {
            AssetViewModel FindAssetViewModel(uint assetId)
            {
                return App.Current.Assets.RegisteredAssets.FirstOrDefault(
                    asset => asset.AssetId == assetId);
            }

            foreach (var entry in commands)
            {
                if (entry.Type == EditorCommandType.AddMaterial)
                {
                    ref readonly AddMaterialCommand cmd = ref entry.As<AddMaterialCommand>();

                    var materialVm = new TerrainMaterialViewModel(cmd.MaterialId, TerrainMaterials)
                    {
                        Name = $"Material {cmd.MaterialId}",
                        AlbedoTexture = FindAssetViewModel(cmd.AlbedoTextureAssetId),
                        NormalTexture = FindAssetViewModel(cmd.NormalTextureAssetId),
                        DisplacementTexture = FindAssetViewModel(cmd.DisplacementTextureAssetId),
                        AoTexture = FindAssetViewModel(cmd.AoTextureAssetId),
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
                else if (entry.Type == EditorCommandType.AddObject)
                {
                    ref readonly AddObjectCommand cmd = ref entry.As<AddObjectCommand>();

                    ObjectIds.Add(cmd.ObjectId);
                }
                else if (entry.Type == EditorCommandType.DeleteObject)
                {
                    ref readonly DeleteObjectCommand cmd = ref entry.As<DeleteObjectCommand>();

                    ObjectIds.Remove(cmd.ObjectId);
                }
            }

            AddMaterialCommand.NotifyCanExecuteChanged();
            for (int i = 0; i < TerrainMaterials.Count; i++)
            {
                var materialVm = TerrainMaterials[i];
                materialVm.CanSetMaterialProperties = i > 0;
                materialVm.MoveMaterialUpCommand.NotifyCanExecuteChanged();
                materialVm.MoveMaterialDownCommand.NotifyCanExecuteChanged();
                materialVm.DeleteMaterialCommand.NotifyCanExecuteChanged();
            }
        }

        internal int GetMaterialIndex(TerrainMaterialViewModel materialVm)
            => TerrainMaterials.IndexOf(materialVm);
    }
}