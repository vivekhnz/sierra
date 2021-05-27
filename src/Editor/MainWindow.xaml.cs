using Microsoft.Win32;
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
        bool isUiInitialized = false;

        public MainWindow()
        {
            InitializeComponent();

            EditorCore.TransactionPublished += OnTransactionPublished;

            var cvsTextureFileAssets = (CollectionViewSource)FindResource("TextureFileAssets");
            cvsTextureFileAssets.Filter += EditorAssetsViewModel.BuildAssetFilter(
                new[] { AssetRegistrationType.File }, new[] { AssetType.Texture });

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

        private void rockTransformSlider_ValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            if (!isUiInitialized) return;

            EditorCore.SetRockTransform(
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

                    lbMaterials.SelectedIndex = lbMaterials.Items.Count - 1;
                }
                else if (entry.Type == EditorCommandType.DeleteMaterial)
                {
                    ref readonly DeleteMaterialCommand cmd = ref entry.As<DeleteMaterialCommand>();

                    if (lbMaterials.Items.Count > 0)
                    {
                        lbMaterials.SelectedIndex = (int)cmd.Index;
                    }
                }
                else if (entry.Type == EditorCommandType.SwapMaterial)
                {
                    ref readonly SwapMaterialCommand cmd = ref entry.As<SwapMaterialCommand>();

                    lbMaterials.SelectedIndex = (int)cmd.IndexB;
                }
            }
        }
    }
}