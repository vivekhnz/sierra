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
        bool isUiInitialized = false;
        DispatcherTimer updateUiTimer;
        Dictionary<RadioButton, EditorTool> editorToolByToolButtons;

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

        public MainWindow()
        {
            EngineInterop.InitializeEngine();
            InitializeComponent();

            editorToolByToolButtons = new Dictionary<RadioButton, EditorTool>
            {
                [rbEditorToolRaiseTerrain] = EditorTool.RaiseTerrain,
                [rbEditorToolLowerTerrain] = EditorTool.LowerTerrain
            };

            updateUiTimer = new DispatcherTimer(DispatcherPriority.Send)
            {
                Interval = TimeSpan.FromMilliseconds(50)
            };
            updateUiTimer.Tick += updateUiTimer_Tick;
            updateUiTimer.Start();

            cbMaterialAlbedoTexture.ItemsSource = textureAssetIdToFilename.Values;
            cbMaterialNormalTexture.ItemsSource = textureAssetIdToFilename.Values;
            cbMaterialAoTexture.ItemsSource = textureAssetIdToFilename.Values;
            cbMaterialDisplacementTexture.ItemsSource = textureAssetIdToFilename.Values;

            UpdateMaterialDetails(0);

            isUiInitialized = true;
        }

        private void Window_Closing(object sender, CancelEventArgs e)
        {
            sceneViewport.Dispose();
            heightmapPreviewViewport.Dispose();
            EngineInterop.Shutdown();
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
                EngineInterop.LoadHeightmapTexture(ofd.FileName);
            }
        }

        private void brushRadiusSlider_ValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            EngineInterop.State.BrushRadius = (float)brushRadiusSlider.Value;
        }

        private void brushFalloffSlider_ValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            EngineInterop.State.BrushFalloff = (float)brushFalloffSlider.Value;
        }

        private void lightDirectionSlider_ValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            EngineInterop.State.LightDirection = (float)lightDirectionSlider.Value;
        }

        private void materialTextureSizeSlider_ValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            if (!isUiInitialized || lbMaterials.SelectedIndex < 0) return;

            float value = (float)materialTextureSizeSlider.Value;
            EngineInterop.SetMaterialTextureSize(lbMaterials.SelectedIndex, value);
        }

        private void materialRampParamsSlider_ValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            if (!isUiInitialized || lbMaterials.SelectedIndex < 0) return;

            if (sender == materialSlopeStartSlider)
            {
                EngineInterop.SetMaterialSlopeStart(lbMaterials.SelectedIndex, (float)materialSlopeStartSlider.Value);
            }
            else if (sender == materialSlopeEndSlider)
            {
                EngineInterop.SetMaterialSlopeEnd(lbMaterials.SelectedIndex, (float)materialSlopeEndSlider.Value);
            }
            else if (sender == materialAltitudeStartSlider)
            {
                EngineInterop.SetMaterialAltitudeStart(lbMaterials.SelectedIndex, (float)materialAltitudeStartSlider.Value);
            }
            else if (sender == materialAltitudeEndSlider)
            {
                EngineInterop.SetMaterialAltitudeEnd(lbMaterials.SelectedIndex, (float)materialAltitudeEndSlider.Value);
            }
        }

        private void OnEditorToolButtonSelected(object sender, RoutedEventArgs e)
        {
            if (editorToolByToolButtons == null) return;

            var senderBtn = sender as RadioButton;
            if (senderBtn == null) return;

            EngineInterop.State.CurrentTool = editorToolByToolButtons[senderBtn];
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
            uint textureAssetId = 0;
            foreach (var kvp in textureAssetIdToFilename)
            {
                if (kvp.Value == textureFilename)
                {
                    textureAssetId = kvp.Key;
                    break;
                }
            }
            if (textureAssetId == 0) return;

            if (dropdown == cbMaterialAlbedoTexture)
            {
                EngineInterop.SetMaterialAlbedoTexture(lbMaterials.SelectedIndex, textureAssetId);
            }
            else if (dropdown == cbMaterialNormalTexture)
            {
                EngineInterop.SetMaterialNormalTexture(lbMaterials.SelectedIndex, textureAssetId);
            }
            else if (dropdown == cbMaterialDisplacementTexture)
            {
                EngineInterop.SetMaterialDisplacementTexture(lbMaterials.SelectedIndex, textureAssetId);
            }
            else if (dropdown == cbMaterialAoTexture)
            {
                EngineInterop.SetMaterialAoTexture(lbMaterials.SelectedIndex, textureAssetId);
            }
        }

        private void updateUiTimer_Tick(object sender, EventArgs e)
        {
            if (brushRadiusSlider.Value != EngineInterop.State.BrushRadius)
            {
                brushRadiusSlider.Value = EngineInterop.State.BrushRadius;
            }
            if (brushFalloffSlider.Value != EngineInterop.State.BrushFalloff)
            {
                brushFalloffSlider.Value = EngineInterop.State.BrushFalloff;
            }

            foreach (var kvp in editorToolByToolButtons)
            {
                bool shouldBeSelected = kvp.Value == EngineInterop.State.CurrentTool;
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

        private void UpdateMaterialDetails(int selectedMaterialIndex)
        {
            var props = EngineInterop.GetMaterialProperties(selectedMaterialIndex);

            cbMaterialAlbedoTexture.SelectedItem = textureAssetIdToFilename[props.albedoTextureAssetId];
            cbMaterialNormalTexture.SelectedItem = textureAssetIdToFilename[props.normalTextureAssetId];
            cbMaterialDisplacementTexture.SelectedItem = textureAssetIdToFilename[props.displacementTextureAssetId];
            cbMaterialAoTexture.SelectedItem = textureAssetIdToFilename[props.aoTextureAssetId];
            materialTextureSizeSlider.Value = props.textureSizeInWorldUnits;

            if (selectedMaterialIndex == 0)
            {
                materialSlopeStartSlider.IsEnabled = false;
                materialSlopeStartSlider.Opacity = 0.2;
                materialSlopeStartSlider.Value = 0;

                materialSlopeEndSlider.IsEnabled = false;
                materialSlopeEndSlider.Opacity = 0.2;
                materialSlopeEndSlider.Value = 0;

                materialAltitudeStartSlider.IsEnabled = false;
                materialAltitudeStartSlider.Opacity = 0.2;
                materialAltitudeStartSlider.Value = 0;

                materialAltitudeEndSlider.IsEnabled = false;
                materialAltitudeEndSlider.Opacity = 0.2;
                materialAltitudeEndSlider.Value = 0;
            }
            else
            {
                materialSlopeStartSlider.IsEnabled = true;
                materialSlopeStartSlider.Opacity = 1;
                materialSlopeStartSlider.Value = props.slopeStart;

                materialSlopeEndSlider.IsEnabled = true;
                materialSlopeEndSlider.Opacity = 1;
                materialSlopeEndSlider.Value = props.slopeEnd;

                materialAltitudeStartSlider.IsEnabled = true;
                materialAltitudeStartSlider.Opacity = 1;
                materialAltitudeStartSlider.Value = props.altitudeStart;

                materialAltitudeEndSlider.IsEnabled = true;
                materialAltitudeEndSlider.Opacity = 1;
                materialAltitudeEndSlider.Value = props.altitudeEnd;
            }
        }
    }
}
