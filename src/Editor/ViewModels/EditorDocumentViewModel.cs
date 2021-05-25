using System.Collections.ObjectModel;
using System.Linq;
using Terrain.Editor.Core;
using Terrain.Editor.Utilities;

namespace Terrain.Editor.ViewModels
{
    public class EditorDocumentViewModel : ViewModelBase
    {
        const int maxMaterialCount = 8;

        private readonly DelegateCommandFactory<TerrainMaterialViewModel> moveMaterialUpCommandFactory;
        private readonly DelegateCommandFactory<TerrainMaterialViewModel> moveMaterialDownCommandFactory;
        private readonly DelegateCommandFactory<TerrainMaterialViewModel> deleteMaterialCommandFactory;

        public ObservableCollection<TerrainMaterialViewModel> TerrainMaterials { get; private set; }
            = new ObservableCollection<TerrainMaterialViewModel>();

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
            moveMaterialUpCommandFactory = new DelegateCommandFactory<TerrainMaterialViewModel>(
                materialVm =>
                {
                    if (materialVm == null) return;

                    int index = TerrainMaterials.IndexOf(materialVm);
                    if (index == -1) return;

                    EditorCore.SwapMaterial(index, index - 1);
                },
                materialVm => materialVm != null && TerrainMaterials.IndexOf(materialVm) > 0);
            moveMaterialDownCommandFactory = new DelegateCommandFactory<TerrainMaterialViewModel>(
                materialVm =>
                {
                    if (materialVm == null) return;

                    int index = TerrainMaterials.IndexOf(materialVm);
                    if (index == -1) return;

                    EditorCore.SwapMaterial(index, index + 1);
                },
                materialVm => materialVm != null
                    && TerrainMaterials.IndexOf(materialVm) < TerrainMaterials.Count - 1);
            deleteMaterialCommandFactory = new DelegateCommandFactory<TerrainMaterialViewModel>(
                materialVm =>
                {
                    if (materialVm == null) return;

                    int index = TerrainMaterials.IndexOf(materialVm);
                    if (index == -1) return;

                    EditorCore.DeleteMaterial(index);
                },
                materialVm => materialVm != null && TerrainMaterials.IndexOf(materialVm) != -1);
        }

        internal void OnTransactionPublished(EditorCommandList commands)
        {
            foreach (var entry in commands)
            {
                if (entry.Type == EditorCommandType.AddMaterial)
                {
                    ref readonly AddMaterialCommand cmd = ref entry.As<AddMaterialCommand>();

                    var materialVm = new TerrainMaterialViewModel(
                        moveMaterialUpCommandFactory, moveMaterialDownCommandFactory,
                        deleteMaterialCommandFactory)
                    {
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

            AddMaterialCommand.NotifyCanExecuteChanged();
            moveMaterialUpCommandFactory.NotifyCanExecuteChanged();
            moveMaterialDownCommandFactory.NotifyCanExecuteChanged();
            deleteMaterialCommandFactory.NotifyCanExecuteChanged();
        }
    }
}