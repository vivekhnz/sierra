using Microsoft.Win32;
using System;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
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

        bool isUiInitialized = false;
        DispatcherTimer updateUiTimer;

        private int prevAssetCount;

        public MainWindow()
        {
            EditorPlatform.Initialize();
            InitializeComponent();

            EditorCore.TransactionPublished += OnTransactionPublished;

            editorUiState = (EditorUiStateViewModel)FindResource("EditorUiState");
            editorAssets = (EditorAssetsViewModel)FindResource("EditorAssets");

            var cvsTextureFileAssets = (CollectionViewSource)FindResource("TextureFileAssets");
            cvsTextureFileAssets.Filter += EditorAssetsViewModel.BuildAssetFilter(
                new[] { AssetRegistrationType.File }, new[] { AssetType.Texture });

            updateUiTimer = new DispatcherTimer(DispatcherPriority.Send)
            {
                Interval = TimeSpan.FromMilliseconds(50)
            };
            updateUiTimer.Tick += updateUiTimer_Tick;
            updateUiTimer.Start();

            UpdateMaterialDetails(0);

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
            lbMaterials.Items.RemoveAt(selectedMaterialIndex);

            if (lbMaterials.Items.Count > 0)
            {
                lbMaterials.SelectedIndex = selectedMaterialIndex;
            }
        }

        private void btnMoveMaterialUp_Click(object sender, RoutedEventArgs e)
        {
            int selectedMaterialIndex = lbMaterials.SelectedIndex;
            if (selectedMaterialIndex < 0) return;

            EditorCore.SwapMaterial(selectedMaterialIndex, selectedMaterialIndex - 1);
            var temp = lbMaterials.Items[selectedMaterialIndex];
            lbMaterials.Items[selectedMaterialIndex] = lbMaterials.Items[selectedMaterialIndex - 1];
            lbMaterials.Items[selectedMaterialIndex - 1] = temp;
            lbMaterials.SelectedIndex = selectedMaterialIndex - 1;
        }

        private void btnMoveMaterialDown_Click(object sender, RoutedEventArgs e)
        {
            int selectedMaterialIndex = lbMaterials.SelectedIndex;
            if (selectedMaterialIndex < 0) return;

            EditorCore.SwapMaterial(selectedMaterialIndex, selectedMaterialIndex + 1);
            var temp = lbMaterials.Items[selectedMaterialIndex];
            lbMaterials.Items[selectedMaterialIndex] = lbMaterials.Items[selectedMaterialIndex + 1];
            lbMaterials.Items[selectedMaterialIndex + 1] = temp;
            lbMaterials.SelectedIndex = selectedMaterialIndex + 1;
        }

        private void OnMaterialParameterSliderValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            if (!isUiInitialized || lbMaterials.SelectedIndex < 0) return;

            EditorCore.SetMaterialProperties(lbMaterials.SelectedIndex,
                (float)materialTextureSizeSlider.Value,
                (float)materialSlopeStartSlider.Value, (float)materialSlopeEndSlider.Value,
                (float)materialAltitudeStartSlider.Value, (float)materialAltitudeEndSlider.Value);
        }

        private void OnMaterialSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (!isUiInitialized || lbMaterials.SelectedIndex < 0) return;

            UpdateMaterialDetails(lbMaterials.SelectedIndex);
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

            if (prevAssetCount == 0)
            {
                int assetCount = TerrainEngine.GetRegisteredAssetCount();
                if (assetCount != prevAssetCount)
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
                    EditorCore.AddMaterial(new TerrainMaterialProperties
                    {
                        AlbedoTextureAssetId = GetTextureAssetId("rock_albedo.jpg"),
                        NormalTextureAssetId = GetTextureAssetId("rock_normal.jpg"),
                        DisplacementTextureAssetId = GetTextureAssetId("rock_displacement.tga"),
                        AoTextureAssetId = GetTextureAssetId("rock_ao.tga"),
                        TextureSizeInWorldUnits = 13.0f,
                        SlopeStart = 0.2f,
                        SlopeEnd = 0.4f,
                        AltitudeStart = 0,
                        AltitudeEnd = 0.001f
                    });
                    EditorCore.AddMaterial(new TerrainMaterialProperties
                    {
                        AlbedoTextureAssetId = GetTextureAssetId("snow_albedo.jpg"),
                        NormalTextureAssetId = GetTextureAssetId("snow_normal.jpg"),
                        DisplacementTextureAssetId = GetTextureAssetId("snow_displacement.tga"),
                        AoTextureAssetId = GetTextureAssetId("snow_ao.tga"),
                        TextureSizeInWorldUnits = 2.0f,
                        SlopeStart = 0.4f,
                        SlopeEnd = 0.2f,
                        AltitudeStart = 0.25f,
                        AltitudeEnd = 0.28f
                    });

                    prevAssetCount = assetCount;
                }
            }
        }

        private void UpdateMaterialDetails(int selectedMaterialIndex)
        {
            var props = EditorCore.GetMaterialProperties(selectedMaterialIndex);

            bool isFirstMaterial = selectedMaterialIndex == 0;
            bool isLastMaterial = selectedMaterialIndex == lbMaterials.Items.Count - 1;

            btnMoveMaterialUp.IsEnabled = !isFirstMaterial;
            btnMoveMaterialUp.Opacity = btnMoveMaterialUp.IsEnabled ? 1 : 0.2;
            btnMoveMaterialDown.IsEnabled = !isLastMaterial;
            btnMoveMaterialDown.Opacity = btnMoveMaterialDown.IsEnabled ? 1 : 0.2;

            AssetViewModel FindAssetViewModel(uint assetId)
            {
                return editorAssets.RegisteredAssets.FirstOrDefault(
                    asset => asset.AssetId == assetId);
            }

            cbMaterialAlbedoTexture.SelectedItem = FindAssetViewModel(props.AlbedoTextureAssetId);
            cbMaterialNormalTexture.SelectedItem = FindAssetViewModel(props.NormalTextureAssetId);
            cbMaterialDisplacementTexture.SelectedItem = FindAssetViewModel(props.DisplacementTextureAssetId);
            cbMaterialAoTexture.SelectedItem = FindAssetViewModel(props.AoTextureAssetId);
            materialTextureSizeSlider.Value = props.TextureSizeInWorldUnits;

            materialSlopeStartSlider.Value = props.SlopeStart;
            materialSlopeEndSlider.Value = props.SlopeEnd;
            materialAltitudeStartSlider.Value = props.AltitudeStart;
            materialAltitudeEndSlider.Value = props.AltitudeEnd;

            if (selectedMaterialIndex == 0)
            {
                materialSlopeStartSlider.IsEnabled = false;
                materialSlopeStartSlider.Opacity = 0.2;

                materialSlopeEndSlider.IsEnabled = false;
                materialSlopeEndSlider.Opacity = 0.2;

                materialAltitudeStartSlider.IsEnabled = false;
                materialAltitudeStartSlider.Opacity = 0.2;

                materialAltitudeEndSlider.IsEnabled = false;
                materialAltitudeEndSlider.Opacity = 0.2;
            }
            else
            {
                materialSlopeStartSlider.IsEnabled = true;
                materialSlopeStartSlider.Opacity = 1;

                materialSlopeEndSlider.IsEnabled = true;
                materialSlopeEndSlider.Opacity = 1;

                materialAltitudeStartSlider.IsEnabled = true;
                materialAltitudeStartSlider.Opacity = 1;

                materialAltitudeEndSlider.IsEnabled = true;
                materialAltitudeEndSlider.Opacity = 1;
            }
        }

        private void OnTransactionPublished(Span<byte> commandBuffer)
        {
            int offset = 0;

            ref T Pop<T>(Span<byte> span)
                where T : struct
            {
                int size = Unsafe.SizeOf<T>();
                Span<byte> sliceSpan = span.Slice(offset, size);
                offset += size;

                return ref MemoryMarshal.AsRef<T>(sliceSpan);
            }

            while (offset < commandBuffer.Length)
            {
                switch (Pop<EditorCommandType>(commandBuffer))
                {
                    case EditorCommandType.AddMaterial:
                        {
                            ref readonly var cmd = ref Pop<AddMaterialCommand>(commandBuffer);

                            lbMaterials.Items.Add($"Material {lbMaterials.Items.Count + 1}");
                            btnAddMaterial.IsEnabled = lbMaterials.Items.Count < maxMaterialCount;
                            lbMaterials.SelectedIndex = lbMaterials.Items.Count - 1;

                            break;
                        }
                    case EditorCommandType.DeleteMaterial:
                        {
                            ref readonly var cmd = ref Pop<DeleteMaterialCommand>(commandBuffer);
                            break;
                        }
                    case EditorCommandType.SwapMaterial:
                        {
                            ref readonly var cmd = ref Pop<SwapMaterialCommand>(commandBuffer);
                            break;
                        }
                    case EditorCommandType.SetMaterialTexture:
                        {
                            ref readonly var cmd = ref Pop<SetMaterialTextureCommand>(commandBuffer);
                            break;
                        }
                    case EditorCommandType.SetMaterialProperties:
                        {
                            ref readonly var cmd = ref Pop<SetMaterialPropertiesCommand>(commandBuffer);
                            break;
                        }
                }
            }
        }
    }
}