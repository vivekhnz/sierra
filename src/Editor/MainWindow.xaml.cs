using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using Terrain.Editor.Core;
using Terrain.Editor.Engine;
using Terrain.Editor.Platform;

namespace Terrain.Editor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        const int maxMaterialCount = 8;

        bool isUiInitialized = false;
        DispatcherTimer updateUiTimer;
        Dictionary<RadioButton, TerrainBrushTool> editorToolByToolButtons;

        private int prevAssetCount;
        private readonly static Dictionary<uint, string> textureAssetIdToFilename
            = new Dictionary<uint, string>
            {
                [0] = null
            };
        private static Dictionary<string, uint> textureFilenameToAssetId
            = new Dictionary<string, uint>();

        public MainWindow()
        {
            EditorPlatform.Initialize();
            InitializeComponent();

            editorToolByToolButtons = new Dictionary<RadioButton, TerrainBrushTool>
            {
                [rbEditorToolRaiseTerrain] = TerrainBrushTool.Raise,
                [rbEditorToolLowerTerrain] = TerrainBrushTool.Lower,
                [rbEditorToolFlattenTerrain] = TerrainBrushTool.Flatten,
                [rbEditorToolSmoothTerrain] = TerrainBrushTool.Smooth
            };

            updateUiTimer = new DispatcherTimer(DispatcherPriority.Send)
            {
                Interval = TimeSpan.FromMilliseconds(50)
            };
            updateUiTimer.Tick += updateUiTimer_Tick;
            updateUiTimer.Start();

            cbMaterialAlbedoTexture.ItemsSource = textureFilenameToAssetId.Keys;
            cbMaterialNormalTexture.ItemsSource = textureFilenameToAssetId.Keys;
            cbMaterialAoTexture.ItemsSource = textureFilenameToAssetId.Keys;
            cbMaterialDisplacementTexture.ItemsSource = textureFilenameToAssetId.Keys;

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

        private void OnBrushParameterSliderValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            if (!isUiInitialized) return;

            ref EditorUiState uiState = ref EditorCore.GetUiState();
            uiState.TerrainBrushRadius = (float)brushRadiusSlider.Value;
            uiState.TerrainBrushFalloff = (float)brushFalloffSlider.Value;
            uiState.TerrainBrushStrength = (float)brushStrengthSlider.Value;
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

        private void lightDirectionSlider_ValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            ref EditorUiState uiState = ref EditorCore.GetUiState();
            uiState.SceneLightDirection = (float)lightDirectionSlider.Value;
        }

        private void btnAddMaterial_Click(object sender, RoutedEventArgs e)
        {
            AddMaterial(new TerrainMaterialProperties
            {
                AlbedoTextureAssetId = textureFilenameToAssetId["ground_albedo.bmp"],
                NormalTextureAssetId = textureFilenameToAssetId["ground_normal.bmp"],
                DisplacementTextureAssetId = textureFilenameToAssetId["ground_displacement.tga"],
                AoTextureAssetId = textureFilenameToAssetId["ground_ao.tga"],
                TextureSizeInWorldUnits = 2.5f,
                SlopeStart = 0.0f,
                SlopeEnd = 0.0f,
                AltitudeStart = 0.0f,
                AltitudeEnd = 0.0f
            });
            lbMaterials.SelectedIndex = lbMaterials.Items.Count - 1;
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

        private void OnEditorToolButtonSelected(object sender, RoutedEventArgs e)
        {
            if (editorToolByToolButtons == null) return;

            var senderBtn = sender as RadioButton;
            if (senderBtn == null) return;

            ref EditorUiState uiState = ref EditorCore.GetUiState();
            uiState.TerrainBrushTool = editorToolByToolButtons[senderBtn];
        }

        private void OnMaterialSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (!isUiInitialized || lbMaterials.SelectedIndex < 0) return;

            UpdateMaterialDetails(lbMaterials.SelectedIndex);
        }

        private void OnMaterialTextureComboBoxSelectionChanged(object sender,
            SelectionChangedEventArgs e)
        {
            if (!(sender is ComboBox dropdown) || lbMaterials.SelectedIndex < 0) return;

            string textureFilename = dropdown.SelectedItem.ToString();
            if (!textureFilenameToAssetId.TryGetValue(textureFilename, out uint textureAssetId))
            {
                return;
            }

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
            EditorCore.SetMaterialTexture(lbMaterials.SelectedIndex, textureType, textureAssetId);
        }

        private void updateUiTimer_Tick(object sender, EventArgs e)
        {
            ref EditorUiState uiState = ref EditorCore.GetUiState();

            if (brushRadiusSlider.Value != uiState.TerrainBrushRadius)
            {
                brushRadiusSlider.Value = uiState.TerrainBrushRadius;
            }
            if (brushFalloffSlider.Value != uiState.TerrainBrushFalloff)
            {
                brushFalloffSlider.Value = uiState.TerrainBrushFalloff;
            }
            if (brushStrengthSlider.Value != uiState.TerrainBrushStrength)
            {
                brushStrengthSlider.Value = uiState.TerrainBrushStrength;
            }

            foreach (var kvp in editorToolByToolButtons)
            {
                bool shouldBeSelected = kvp.Value == uiState.TerrainBrushTool;
                if (shouldBeSelected && kvp.Key.IsChecked != true)
                {
                    kvp.Key.IsChecked = true;
                }
                else if (!shouldBeSelected && kvp.Key.IsChecked != false)
                {
                    kvp.Key.IsChecked = false;
                }
            }

            int assetCount = TerrainEngine.GetRegisteredAssetCount();
            if (assetCount != prevAssetCount)
            {
                var registeredAssets = TerrainEngine.GetRegisteredAssets();
                foreach (var asset in registeredAssets)
                {
                    if (asset.RegistrationType == AssetRegistrationType.File
                        && asset.GetAssetType() == AssetType.Texture)
                    {
                        var fileState = asset.GetFileState();
                        textureAssetIdToFilename[asset.Id] = fileState.RelativePath;
                        textureFilenameToAssetId[fileState.RelativePath] = asset.Id;
                    }
                }

                if (prevAssetCount == 0)
                {
                    AddMaterial(new TerrainMaterialProperties
                    {
                        AlbedoTextureAssetId = textureFilenameToAssetId["ground_albedo.bmp"],
                        NormalTextureAssetId = textureFilenameToAssetId["ground_normal.bmp"],
                        DisplacementTextureAssetId = textureFilenameToAssetId["ground_displacement.tga"],
                        AoTextureAssetId = textureFilenameToAssetId["ground_ao.tga"],
                        TextureSizeInWorldUnits = 2.5f,
                        SlopeStart = 0.0f,
                        SlopeEnd = 0.0f,
                        AltitudeStart = 0.0f,
                        AltitudeEnd = 0.0f
                    });
                    AddMaterial(new TerrainMaterialProperties
                    {
                        AlbedoTextureAssetId = textureFilenameToAssetId["rock_albedo.jpg"],
                        NormalTextureAssetId = textureFilenameToAssetId["rock_normal.jpg"],
                        DisplacementTextureAssetId = textureFilenameToAssetId["rock_displacement.tga"],
                        AoTextureAssetId = textureFilenameToAssetId["rock_ao.tga"],
                        TextureSizeInWorldUnits = 13.0f,
                        SlopeStart = 0.2f,
                        SlopeEnd = 0.4f,
                        AltitudeStart = 0,
                        AltitudeEnd = 0.001f
                    });
                    AddMaterial(new TerrainMaterialProperties
                    {
                        AlbedoTextureAssetId = textureFilenameToAssetId["snow_albedo.jpg"],
                        NormalTextureAssetId = textureFilenameToAssetId["snow_normal.jpg"],
                        DisplacementTextureAssetId = textureFilenameToAssetId["snow_displacement.tga"],
                        AoTextureAssetId = textureFilenameToAssetId["snow_ao.tga"],
                        TextureSizeInWorldUnits = 2.0f,
                        SlopeStart = 0.4f,
                        SlopeEnd = 0.2f,
                        AltitudeStart = 0.25f,
                        AltitudeEnd = 0.28f
                    });
                }

                prevAssetCount = assetCount;
            }
        }

        private void AddMaterial(TerrainMaterialProperties props)
        {
            EditorCore.AddMaterial(props);
            lbMaterials.Items.Add($"Material {lbMaterials.Items.Count + 1}");

            btnAddMaterial.IsEnabled = lbMaterials.Items.Count < maxMaterialCount;
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

            cbMaterialAlbedoTexture.SelectedItem = textureAssetIdToFilename[props.AlbedoTextureAssetId];
            cbMaterialNormalTexture.SelectedItem = textureAssetIdToFilename[props.NormalTextureAssetId];
            cbMaterialDisplacementTexture.SelectedItem = textureAssetIdToFilename[props.DisplacementTextureAssetId];
            cbMaterialAoTexture.SelectedItem = textureAssetIdToFilename[props.AoTextureAssetId];
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
    }
}
