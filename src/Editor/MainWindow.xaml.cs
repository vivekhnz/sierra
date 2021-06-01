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

        private void cbObjects_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
        {
            if (!isUiInitialized) return;
            if (cbObjects.SelectedIndex < 0) return;

            var transform = EditorCore.GetObjectTransform((uint)cbObjects.SelectedIndex + 1);
            rockPositionXSlider.Value = transform.Position.X;
            rockPositionYSlider.Value = transform.Position.Y;
            rockPositionZSlider.Value = transform.Position.Z;
            rockRotationXSlider.Value = transform.Rotation.X;
            rockRotationYSlider.Value = transform.Rotation.Y;
            rockRotationZSlider.Value = transform.Rotation.Z;
            rockScaleXSlider.Value = transform.Scale.X;
            rockScaleYSlider.Value = transform.Scale.Y;
            rockScaleZSlider.Value = transform.Scale.Z;
        }

        private void rockTransformSlider_ValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            if (!isUiInitialized) return;
            if (cbObjects.SelectedIndex < 0) return;

            EditorCore.SetObjectTransform(
                (uint)cbObjects.SelectedIndex + 1,
                (float)rockPositionXSlider.Value,
                (float)rockPositionYSlider.Value,
                (float)rockPositionZSlider.Value,
                (float)rockRotationXSlider.Value,
                (float)rockRotationYSlider.Value,
                (float)rockRotationZSlider.Value,
                (float)rockScaleXSlider.Value,
                (float)rockScaleYSlider.Value,
                (float)rockScaleZSlider.Value);
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

                    cbObjects.Items.Add($"Object {cmd.ObjectId}");
                    cbObjects.SelectedIndex = cbObjects.Items.Count - 1;
                }
            }
        }
    }
}