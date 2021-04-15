using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using Terrain.Engine.Interop;
using Terrain.Engine.Interop.Proxy;

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
        Dictionary<RadioButton, EditorToolProxy> editorToolByToolButtons;

        private const uint textureAssetTypeId = 3;
        private readonly static Dictionary<uint, string> textureAssetIdToFilename
            = new Dictionary<uint, string>
            {
                [0 | (textureAssetTypeId << 28)] = "ground_albedo.bmp",
                [1 | (textureAssetTypeId << 28)] = "ground_normal.bmp",
                [2 | (textureAssetTypeId << 28)] = "ground_displacement.tga",
                [3 | (textureAssetTypeId << 28)] = "ground_ao.tga",
                [4 | (textureAssetTypeId << 28)] = "rock_albedo.jpg",
                [5 | (textureAssetTypeId << 28)] = "rock_normal.jpg",
                [6 | (textureAssetTypeId << 28)] = "rock_displacement.tga",
                [7 | (textureAssetTypeId << 28)] = "rock_ao.tga",
                [8 | (textureAssetTypeId << 28)] = "snow_albedo.jpg",
                [9 | (textureAssetTypeId << 28)] = "snow_normal.jpg",
                [10 | (textureAssetTypeId << 28)] = "snow_displacement.tga",
                [11 | (textureAssetTypeId << 28)] = "snow_ao.tga"
            };
        private static Dictionary<string, uint> textureFilenameToAssetId
            = new Dictionary<string, uint>();

        public MainWindow()
        {
            foreach (var kvp in textureAssetIdToFilename)
            {
                textureFilenameToAssetId[kvp.Value] = kvp.Key;
            }

            EditorPlatform.Initialize();
            InitializeComponent();

            AddMaterial(new MaterialProps
            {
                albedoTextureAssetId = textureFilenameToAssetId["ground_albedo.bmp"],
                normalTextureAssetId = textureFilenameToAssetId["ground_normal.bmp"],
                displacementTextureAssetId = textureFilenameToAssetId["ground_displacement.tga"],
                aoTextureAssetId = textureFilenameToAssetId["ground_ao.tga"],
                textureSizeInWorldUnits = 2.5f,
                slopeStart = 0.0f,
                slopeEnd = 0.0f,
                altitudeStart = 0.0f,
                altitudeEnd = 0.0f
            });
            AddMaterial(new MaterialProps
            {
                albedoTextureAssetId = textureFilenameToAssetId["rock_albedo.jpg"],
                normalTextureAssetId = textureFilenameToAssetId["rock_normal.jpg"],
                displacementTextureAssetId = textureFilenameToAssetId["rock_displacement.tga"],
                aoTextureAssetId = textureFilenameToAssetId["rock_ao.tga"],
                textureSizeInWorldUnits = 13.0f,
                slopeStart = 0.2f,
                slopeEnd = 0.4f,
                altitudeStart = 0,
                altitudeEnd = 0.001f
            });
            AddMaterial(new MaterialProps
            {
                albedoTextureAssetId = textureFilenameToAssetId["snow_albedo.jpg"],
                normalTextureAssetId = textureFilenameToAssetId["snow_normal.jpg"],
                displacementTextureAssetId = textureFilenameToAssetId["snow_displacement.tga"],
                aoTextureAssetId = textureFilenameToAssetId["snow_ao.tga"],
                textureSizeInWorldUnits = 2.0f,
                slopeStart = 0.4f,
                slopeEnd = 0.2f,
                altitudeStart = 0.25f,
                altitudeEnd = 0.28f
            });

            editorToolByToolButtons = new Dictionary<RadioButton, EditorToolProxy>
            {
                [rbEditorToolRaiseTerrain] = EditorToolProxy.RaiseTerrain,
                [rbEditorToolLowerTerrain] = EditorToolProxy.LowerTerrain,
                [rbEditorToolFlattenTerrain] = EditorToolProxy.FlattenTerrain,
                [rbEditorToolSmoothTerrain] = EditorToolProxy.SmoothTerrain
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
            sceneViewport.Dispose();
            heightmapPreviewViewport.Dispose();
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
                EditorCore.LoadHeightmapTexture(ofd.FileName);
            }
        }

        private void OnBrushParameterSliderValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            if (!isUiInitialized) return;

            EditorCore.SetBrushParameters(
                (float)brushRadiusSlider.Value,
                (float)brushFalloffSlider.Value,
                (float)brushStrengthSlider.Value);
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
            EditorCore.SetSceneParameters((float)lightDirectionSlider.Value);
        }

        private void btnAddMaterial_Click(object sender, RoutedEventArgs e)
        {
            AddMaterial(new MaterialProps
            {
                albedoTextureAssetId = textureFilenameToAssetId["ground_albedo.bmp"],
                normalTextureAssetId = textureFilenameToAssetId["ground_normal.bmp"],
                displacementTextureAssetId = textureFilenameToAssetId["ground_displacement.tga"],
                aoTextureAssetId = textureFilenameToAssetId["ground_ao.tga"],
                textureSizeInWorldUnits = 2.5f,
                slopeStart = 0.0f,
                slopeEnd = 0.0f,
                altitudeStart = 0.0f,
                altitudeEnd = 0.0f
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

            EditorCore.SetMaterialParameters(lbMaterials.SelectedIndex,
                (float)materialTextureSizeSlider.Value,
                (float)materialSlopeStartSlider.Value, (float)materialSlopeEndSlider.Value,
                (float)materialAltitudeStartSlider.Value, (float)materialAltitudeEndSlider.Value);
        }

        private void OnEditorToolButtonSelected(object sender, RoutedEventArgs e)
        {
            if (editorToolByToolButtons == null) return;

            var senderBtn = sender as RadioButton;
            if (senderBtn == null) return;

            EditorCore.SetBrushTool(editorToolByToolButtons[senderBtn]);
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
            var brushParams = EditorCore.GetBrushParameters();
            if (brushRadiusSlider.Value != brushParams.Radius)
            {
                brushRadiusSlider.Value = brushParams.Radius;
            }
            if (brushFalloffSlider.Value != brushParams.Falloff)
            {
                brushFalloffSlider.Value = brushParams.Falloff;
            }
            if (brushStrengthSlider.Value != brushParams.Strength)
            {
                brushStrengthSlider.Value = brushParams.Strength;
            }

            EditorToolProxy currentTool = EditorCore.GetBrushTool();
            foreach (var kvp in editorToolByToolButtons)
            {
                bool shouldBeSelected = kvp.Value == currentTool;
                if (shouldBeSelected && kvp.Key.IsChecked != true)
                {
                    kvp.Key.IsChecked = true;
                }
                else if (!shouldBeSelected && kvp.Key.IsChecked != false)
                {
                    kvp.Key.IsChecked = false;
                }
            }
        }

        private void AddMaterial(MaterialProps props)
        {
            lbMaterials.Items.Add($"Material {lbMaterials.Items.Count + 1}");
            EditorCore.AddMaterial(props);

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

            cbMaterialAlbedoTexture.SelectedItem = textureAssetIdToFilename[props.albedoTextureAssetId];
            cbMaterialNormalTexture.SelectedItem = textureAssetIdToFilename[props.normalTextureAssetId];
            cbMaterialDisplacementTexture.SelectedItem = textureAssetIdToFilename[props.displacementTextureAssetId];
            cbMaterialAoTexture.SelectedItem = textureAssetIdToFilename[props.aoTextureAssetId];
            materialTextureSizeSlider.Value = props.textureSizeInWorldUnits;

            materialSlopeStartSlider.Value = props.slopeStart;
            materialSlopeEndSlider.Value = props.slopeEnd;
            materialAltitudeStartSlider.Value = props.altitudeStart;
            materialAltitudeEndSlider.Value = props.altitudeEnd;

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
