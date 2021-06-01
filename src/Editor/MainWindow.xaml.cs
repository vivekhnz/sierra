using Microsoft.Win32;
using System.Linq;
using System.Windows;
using System.Windows.Data;
using Terrain.Editor.Core;
using Terrain.Editor.Engine;
using Terrain.Editor.Platform;
using Terrain.Editor.ViewModels;

namespace Terrain.Editor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private readonly CollectionViewSource cvsTerrainMaterials;

        bool isUiInitialized = false;

        public MainWindow()
        {
            InitializeComponent();

            EditorCore.TransactionPublished += OnTransactionPublished;

            var cvsTextureFileAssets = (CollectionViewSource)FindResource("TextureFileAssets");
            cvsTextureFileAssets.Filter += EditorAssetsViewModel.BuildAssetFilter(
                new[] { AssetRegistrationType.File }, new[] { AssetType.Texture });

            cvsTerrainMaterials = (CollectionViewSource)FindResource("TerrainMaterials");

            isUiInitialized = true;
        }

        private void miOpen_Click(object sender, RoutedEventArgs e)
        {
            var ofd = new OpenFileDialog
            {
                Filter = "TGA (*.tga)|*.tga",
                Title = "Open heightmap file"
            };
            if (ofd.ShowDialog() == true)
            {
                uint? heightmapAssetId = EditorCore.GetImportedHeightmapAssetId();
                if (heightmapAssetId.HasValue)
                {
                    EditorPlatform.QueueAssetLoad(heightmapAssetId.Value, ofd.FileName);
                }
            }
        }

        private void OnTransactionPublished(EditorCommandList commands)
        {
            foreach (var entry in commands)
            {
                if (entry.Type == EditorCommandType.AddMaterial)
                {
                    ref readonly AddMaterialCommand cmd = ref entry.As<AddMaterialCommand>();
                    uint materialId = cmd.MaterialId;

                    var materialVm = App.Current.Document.TerrainMaterials.FirstOrDefault(
                        vm => vm.MaterialId == materialId);
                    if (materialVm != null)
                    {
                        cvsTerrainMaterials.View.MoveCurrentTo(materialVm);
                    }
                }
            }
        }
    }
}