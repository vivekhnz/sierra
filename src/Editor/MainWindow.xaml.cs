using System.Linq;
using System.Windows;
using System.Windows.Data;
using Terrain.Editor.Core;
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

        public MainWindow()
        {
            InitializeComponent();

            PreviewKeyDown += (sender, e) =>
            {
                e.Handled = EditorPlatform.IsViewportHovered;
            };

            EditorCore.TransactionPublished += OnTransactionPublished;

            var cvsTextureFileAssets = (CollectionViewSource)FindResource("TextureFileAssets");
            cvsTextureFileAssets.Filter += EditorAssetsViewModel.BuildAssetFilter(
                new[] { AssetRegistrationType.File }, new[] { AssetType.Texture });

            cvsTerrainMaterials = (CollectionViewSource)FindResource("TerrainMaterials");
        }

        private void miOpen_Click(object sender, RoutedEventArgs e)
        {
            // todo: reimplement functionality to import a heightmap
#if false
            var ofd = new OpenFileDialog
            {
                Filter = "TGA (*.tga)|*.tga",
                Title = "Open heightmap file"
            };
            if (ofd.ShowDialog() == true)
            {
                IntPtr heightmapAssetHandle = EditorCore.GetImportedHeightmapAssetHandle();
                if (heightmapAssetHandle != IntPtr.Zero)
                {
                    EditorPlatform.QueueAssetLoad(heightmapAssetHandle, ofd.FileName);
                }
            }
#endif
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
                else if (entry.Type == EditorCommandType.AddObject)
                {
                    ref readonly AddObjectCommand cmd = ref entry.As<AddObjectCommand>();
                    ref EditorUiState state = ref EditorCore.GetUiState();
                    state.SelectedObjectCount = 1;
                    state.SelectedObjectIds[0] = cmd.ObjectId;
                }
            }
        }
    }
}