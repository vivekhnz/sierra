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
    }
}
