using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using Terrain.Engine.Interop;

namespace Terrain.Editor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
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
                const int RESOURCE_ID_TEXTURE_HEIGHTMAP = 0;
                EngineInterop.ResourceManager.ReloadTexture(RESOURCE_ID_TEXTURE_HEIGHTMAP,
                    ofd.FileName, true);
                EngineInterop.State.CurrentHeightmapStatus = HeightmapStatus.Initializing;
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

        private void mat1TextureSizeSlider_ValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            EngineInterop.State.Material1TextureSize = (float)mat1TextureSizeSlider.Value;
        }

        private void mat2TextureSizeSlider_ValueChanged(object sender,
            RoutedPropertyChangedEventArgs<double> e)
        {
            EngineInterop.State.Material2TextureSize = (float)mat2TextureSizeSlider.Value;
        }

        private void OnEditorToolButtonSelected(object sender, RoutedEventArgs e)
        {
            if (editorToolByToolButtons == null) return;

            var senderBtn = sender as RadioButton;
            if (senderBtn == null) return;

            EngineInterop.State.CurrentTool = editorToolByToolButtons[senderBtn];
        }

        private void OnMaterialTextureComboBoxSelectionChanged(object sender,
            SelectionChangedEventArgs e)
        {
            if (!(sender is ComboBox dropdown)) return;

            if (!terrainMaterialTextureSlotByTextureDropdowns.TryGetValue(
                dropdown, out int slot)) return;

            string textureIdAlias = dropdown.SelectedItem.ToString();
            if (!textureIdMap.TryGetValue(textureIdAlias, out int textureId)) return;

            const int RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED = 0;
            int materialHandle = EngineInterop.GraphicsAssetManager.LookupMaterial(
                RESOURCE_ID_MATERIAL_TERRAIN_TEXTURED);
            EngineInterop.GraphicsAssetManager.SetMaterialTexture(materialHandle, slot,
                EngineInterop.Renderer.LookupTexture(textureId));
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
    }
}
