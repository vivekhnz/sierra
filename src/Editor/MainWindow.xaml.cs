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
        Dictionary<ComboBox, int> terrainMaterialTextureSlotByTextureDropdowns;

        private static Dictionary<string, int> textureIdMap = new Dictionary<string, int>
        {
            ["ground_albedo"] = 1,
            ["ground_normal"] = 2,
            ["ground_displacement"] = 3,
            ["ground_ao"] = 4,
            ["rock_albedo"] = 5,
            ["rock_normal"] = 6,
            ["rock_displacement"] = 7,
            ["rock_ao"] = 8
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
            /*
            terrainMaterialTextureSlotByTextureDropdowns = new Dictionary<ComboBox, int>
            {
                [cbMaterialAlbedoTexture] = 1,
                [cbMaterialNormalTexture] = 2,
                [cbMaterialDisplacementTexture] = 3,
                [cbMaterialAOTexture] = 4
            };
            */

            updateUiTimer = new DispatcherTimer(DispatcherPriority.Send)
            {
                Interval = TimeSpan.FromMilliseconds(50)
            };
            updateUiTimer.Tick += updateUiTimer_Tick;
            updateUiTimer.Start();

            /*
            cbMaterialAlbedoTexture.ItemsSource = textureIdMap.Keys;
            cbMaterialNormalTexture.ItemsSource = textureIdMap.Keys;
            cbMaterialAOTexture.ItemsSource = textureIdMap.Keys;
            cbMaterialDisplacementTexture.ItemsSource = textureIdMap.Keys;

            cbMaterialAlbedoTexture.SelectedItem = "ground_albedo";
            cbMaterialNormalTexture.SelectedItem = "ground_normal";
            cbMaterialAOTexture.SelectedItem = "ground_ao";
            cbMaterialDisplacementTexture.SelectedItem = "ground_displacement";
            */

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
            if (!isUiInitialized) return;

            float value = (float)materialTextureSizeSlider.Value;
            switch (cbMaterialSelector.SelectedIndex)
            {
                case 0:
                    EngineInterop.State.Material1TextureSize = value;
                    break;
                case 1:
                    EngineInterop.State.Material2TextureSize = value;
                    break;
                case 2:
                    EngineInterop.State.Material3TextureSize = value;
                    break;
            }
        }

        private void materialRampParamsSlider_ValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            if (!isUiInitialized) return;

            RampParamsProxy ramp = null;
            switch (cbMaterialSelector.SelectedIndex)
            {
                case 1:
                    ramp = EngineInterop.State.Material2RampParams;
                    break;
                case 2:
                    ramp = EngineInterop.State.Material3RampParams;
                    break;
            }

            if (ramp == null) return;

            if (sender == materialSlopeStartSlider)
            {
                ramp.SlopeStart = (float)materialSlopeStartSlider.Value;
            }
            else if (sender == materialSlopeEndSlider)
            {
                ramp.SlopeEnd = (float)materialSlopeEndSlider.Value;
            }
            else if (sender == materialAltitudeStartSlider)
            {
                ramp.AltitudeStart = (float)materialAltitudeStartSlider.Value;
            }
            else if (sender == materialAltitudeEndSlider)
            {
                ramp.AltitudeEnd = (float)materialAltitudeEndSlider.Value;
            }
        }

        private void OnEditorToolButtonSelected(object sender, RoutedEventArgs e)
        {
            if (editorToolByToolButtons == null) return;

            var senderBtn = sender as RadioButton;
            if (senderBtn == null) return;

            EngineInterop.State.CurrentTool = editorToolByToolButtons[senderBtn];
        }

        private void OnMaterialSelectorSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (!isUiInitialized) return;

            UpdateMaterialDetails(cbMaterialSelector.SelectedIndex);
        }

        private void OnMaterialTextureComboBoxSelectionChanged(object sender,
            SelectionChangedEventArgs e)
        {
            if (!(sender is ComboBox dropdown)) return;

            if (!terrainMaterialTextureSlotByTextureDropdowns.TryGetValue(
                dropdown, out int slot)) return;

            string textureIdAlias = dropdown.SelectedItem.ToString();
            if (!textureIdMap.TryGetValue(textureIdAlias, out int textureId)) return;

            /*const int RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED = 0;
            int materialHandle = EngineInterop.GraphicsAssetManager.LookupMaterial(
                RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED);
            EngineInterop.GraphicsAssetManager.SetMaterialTexture(materialHandle, slot,
                EngineInterop.Renderer.LookupTexture(textureId));*/
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
            RampParamsProxy ramp = null;
            switch (selectedMaterialIndex)
            {
                case 0:
                    materialTextureSizeSlider.Value = EngineInterop.State.Material1TextureSize;
                    break;
                case 1:
                    materialTextureSizeSlider.Value = EngineInterop.State.Material2TextureSize;
                    ramp = EngineInterop.State.Material2RampParams;
                    break;
                case 2:
                    materialTextureSizeSlider.Value = EngineInterop.State.Material3TextureSize;
                    ramp = EngineInterop.State.Material3RampParams;
                    break;
            }

            if (ramp == null)
            {
                materialSlopeStartSlider.Value = 0;
                materialSlopeEndSlider.Value = 0;
                materialAltitudeStartSlider.Value = 0;
                materialAltitudeEndSlider.Value = 0;
            }
            else
            {
                materialSlopeStartSlider.Value = ramp.SlopeStart;
                materialSlopeEndSlider.Value = ramp.SlopeEnd;
                materialAltitudeStartSlider.Value = ramp.AltitudeStart;
                materialAltitudeEndSlider.Value = ramp.AltitudeEnd;
            }
        }
    }
}
