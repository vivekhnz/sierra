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
            viewport.Dispose();
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
                EngineInterop.Scene.Terrain.LoadHeightmap(ofd.FileName);
            }
        }
    }
}
