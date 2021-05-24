using Microsoft.Win32;
using System;
using System.ComponentModel;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Threading;
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
        const int maxMaterialCount = 8;

        private EditorUiStateViewModel editorUiState;
        private EditorAssetsViewModel editorAssets;
        private EditorDocumentViewModel editorDocument;

        bool isUiInitialized = false;
        DispatcherTimer updateUiTimer;

        public MainWindow()
        {
            EditorPlatform.Initialize();
            InitializeComponent();

            editorUiState = (EditorUiStateViewModel)FindResource("EditorUiState");
            editorAssets = (EditorAssetsViewModel)FindResource("EditorAssets");
            editorDocument = (EditorDocumentViewModel)FindResource("EditorDocument");

            EditorCore.TransactionPublished += editorDocument.OnTransactionPublished;
            EditorCore.TransactionPublished += OnTransactionPublished;

            var cvsTextureFileAssets = (CollectionViewSource)FindResource("TextureFileAssets");
            cvsTextureFileAssets.Filter += EditorAssetsViewModel.BuildAssetFilter(
                new[] { AssetRegistrationType.File }, new[] { AssetType.Texture });

            updateUiTimer = new DispatcherTimer(DispatcherPriority.Send)
            {
                Interval = TimeSpan.FromMilliseconds(50)
            };
            updateUiTimer.Tick += updateUiTimer_Tick;
            updateUiTimer.Start();

            isUiInitialized = true;
        }

        private void Window_Closing(object sender, CancelEventArgs e)
        {
            EditorPlatform.Shutdown();
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

        private void btnAddMaterial_Click(object sender, RoutedEventArgs e)
        {
            uint GetTextureAssetId(string relativePath)
            {
                var assetVm = editorAssets.RegisteredAssets.FirstOrDefault(
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
        }

        private void btnDeleteMaterial_Click(object sender, RoutedEventArgs e)
        {
            int selectedMaterialIndex = lbMaterials.SelectedIndex;
            if (selectedMaterialIndex < 0) return;

            EditorCore.DeleteMaterial(selectedMaterialIndex);
        }

        private void OnMaterialSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            int selectedMaterialIndex = lbMaterials.SelectedIndex;
            if (!isUiInitialized || selectedMaterialIndex < 0) return;

            var props = EditorCore.GetMaterialProperties(selectedMaterialIndex);

            AssetViewModel FindAssetViewModel(uint assetId)
            {
                return editorAssets.RegisteredAssets.FirstOrDefault(
                    asset => asset.AssetId == assetId);
            }

            cbMaterialAlbedoTexture.SelectedItem = FindAssetViewModel(props.AlbedoTextureAssetId);
            cbMaterialNormalTexture.SelectedItem = FindAssetViewModel(props.NormalTextureAssetId);
            cbMaterialDisplacementTexture.SelectedItem = FindAssetViewModel(props.DisplacementTextureAssetId);
            cbMaterialAoTexture.SelectedItem = FindAssetViewModel(props.AoTextureAssetId);
        }

        private void OnMaterialTextureComboBoxSelectionChanged(object sender,
            SelectionChangedEventArgs e)
        {
            int materialIdx = lbMaterials.SelectedIndex;
            if (!(sender is ComboBox dropdown) || materialIdx < 0) return;
            if (!(dropdown.SelectedItem is AssetViewModel assetVm)) return;

            TerrainMaterialTextureType textureType = TerrainMaterialTextureType.Albedo;
            if (dropdown == cbMaterialAlbedoTexture)
            {
                textureType = TerrainMaterialTextureType.Albedo;
            }
            else if (dropdown == cbMaterialNormalTexture)
            {
                textureType = TerrainMaterialTextureType.Normal;
            }
            else if (dropdown == cbMaterialDisplacementTexture)
            {
                textureType = TerrainMaterialTextureType.Displacement;
            }
            else if (dropdown == cbMaterialAoTexture)
            {
                textureType = TerrainMaterialTextureType.AmbientOcclusion;
            }
            EditorCore.SetMaterialTexture(materialIdx, textureType, assetVm.AssetId);
        }

        private void updateUiTimer_Tick(object sender, EventArgs e)
        {
            editorUiState.CheckForChanges();
            editorAssets.CheckForChanges();
        }

        private void OnTransactionPublished(EditorCommandList commands)
        {
            foreach (var entry in commands)
            {
                if (entry.Type == EditorCommandType.AddMaterial)
                {
                    ref readonly AddMaterialCommand cmd = ref entry.As<AddMaterialCommand>();

                    btnAddMaterial.IsEnabled = lbMaterials.Items.Count < maxMaterialCount;
                    lbMaterials.SelectedIndex = lbMaterials.Items.Count - 1;
                }
                else if (entry.Type == EditorCommandType.DeleteMaterial)
                {
                    ref readonly DeleteMaterialCommand cmd = ref entry.As<DeleteMaterialCommand>();

                    if (lbMaterials.Items.Count > 0)
                    {
                        lbMaterials.SelectedIndex = (int)cmd.Index;
                    }
                    btnAddMaterial.IsEnabled = lbMaterials.Items.Count < maxMaterialCount;
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