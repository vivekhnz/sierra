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
        private readonly CollectionViewSource cvsObjects;

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
            cvsObjects = (CollectionViewSource)FindResource("Objects");
            cvsObjects.View.CurrentChanged += (sender, args) =>
            {
                var objectVm = cvsObjects.View.CurrentItem as EditorObjectViewModel;
                App.Current.UiState.SelectedObjectId = objectVm?.ObjectId ?? 0;
            };

            App.CoreTick += () =>
            {
                var objectVm = cvsObjects.View.CurrentItem as EditorObjectViewModel;
                if (objectVm == null) return;

                tbObjectPositionX.Text = EditorCore.GetObjectProperty(
                    objectVm.ObjectId, ObjectProperty.ObjectPositionX).ToString();
                tbObjectPositionY.Text = EditorCore.GetObjectProperty(
                    objectVm.ObjectId, ObjectProperty.ObjectPositionY).ToString();
                tbObjectPositionZ.Text = EditorCore.GetObjectProperty(
                    objectVm.ObjectId, ObjectProperty.ObjectPositionZ).ToString();
                tbObjectRotationX.Text = EditorCore.GetObjectProperty(
                    objectVm.ObjectId, ObjectProperty.ObjectRotationX).ToString();
                tbObjectRotationY.Text = EditorCore.GetObjectProperty(
                    objectVm.ObjectId, ObjectProperty.ObjectRotationY).ToString();
                tbObjectRotationZ.Text = EditorCore.GetObjectProperty(
                    objectVm.ObjectId, ObjectProperty.ObjectRotationZ).ToString();
                tbObjectScaleX.Text = EditorCore.GetObjectProperty(
                    objectVm.ObjectId, ObjectProperty.ObjectScaleX).ToString();
                tbObjectScaleY.Text = EditorCore.GetObjectProperty(
                    objectVm.ObjectId, ObjectProperty.ObjectScaleY).ToString();
                tbObjectScaleZ.Text = EditorCore.GetObjectProperty(
                    objectVm.ObjectId, ObjectProperty.ObjectScaleZ).ToString();
            };
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
                else if (entry.Type == EditorCommandType.AddObject)
                {
                    ref readonly AddObjectCommand cmd = ref entry.As<AddObjectCommand>();
                    uint objectId = cmd.ObjectId;

                    var objectVm = App.Current.Document.Objects.FirstOrDefault(
                        vm => vm.ObjectId == objectId);
                    if (objectVm != null)
                    {
                        cvsObjects.View.MoveCurrentTo(objectVm);
                    }
                }
            }
        }
    }
}