using Microsoft.Win32;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using Terrain.Editor.Core;
using Terrain.Editor.Engine;
using Terrain.Editor.Platform;
using Terrain.Editor.Utilities;
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

            var selectedObjectRef = new ObjectReference(
                () => App.Current.UiState.SelectedObjectId);
            void Bind(TextBlock target, ObjectProperty prop)
            {
                var editorBinding = target.SetBinding(TextBlock.TextProperty, prop);
                editorBinding.Source = selectedObjectRef;
            }
            Bind(tbObjectPositionX, ObjectProperty.ObjectPositionX);
            Bind(tbObjectPositionY, ObjectProperty.ObjectPositionY);
            Bind(tbObjectPositionZ, ObjectProperty.ObjectPositionZ);
            Bind(tbObjectRotationX, ObjectProperty.ObjectRotationX);
            Bind(tbObjectRotationY, ObjectProperty.ObjectRotationY);
            Bind(tbObjectRotationZ, ObjectProperty.ObjectRotationZ);
            Bind(tbObjectScaleX, ObjectProperty.ObjectScaleX);
            Bind(tbObjectScaleY, ObjectProperty.ObjectScaleY);
            Bind(tbObjectScaleZ, ObjectProperty.ObjectScaleZ);
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