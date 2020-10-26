using Microsoft.Win32;
using System.ComponentModel;
using System.Windows;
using Terrain.Engine.Interop;

namespace Terrain.Editor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            EngineInterop.InitializeEngine();
            InitializeComponent();
        }

        private void Window_Closing(object sender, CancelEventArgs e)
        {
            sceneViewport.Dispose();
            heightmapCompositionViewport.Dispose();
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
            }
        }
    }
}
