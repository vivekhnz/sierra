using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Data;
using Microsoft.Win32;
using Sierra.Core;
using Sierra.Platform;
using Sierra.ViewModels;

namespace Sierra
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private readonly CollectionViewSource cvsTerrainMaterials;
        private string heightmapFilePath;

        public MainWindow()
        {
            InitializeComponent();

            EditorPlatform.AttachToWindow(this);

            EditorCore.TransactionPublished += OnTransactionPublished;
            App.PerformanceCountersUpdated += OnPerfCountersUpdated;

            var cvsTextureFileAssets = (CollectionViewSource)FindResource("TextureFileAssets");
            cvsTextureFileAssets.Filter += EditorAssetsViewModel.BuildAssetFilter(
                new[] { AssetRegistrationType.File }, new[] { AssetType.Texture });

            cvsTerrainMaterials = (CollectionViewSource)FindResource("TerrainMaterials");
        }

        private void miOpen_Click(object sender, RoutedEventArgs e)
        {
            // todo: reimplement functionality to import a heightmap
#if false
            var ofd = new OpenFileDialog
            {
                Filter = "TGA (*.tga)|*.tga",
                Title = "Open heightmap file"
            };
            if (ofd.ShowDialog() == true)
            {
                IntPtr heightmapAssetHandle = EditorCore.GetImportedHeightmapAssetHandle();
                if (heightmapAssetHandle != IntPtr.Zero)
                {
                    EditorPlatform.QueueAssetLoad(heightmapAssetHandle, ofd.FileName);
                }
            }
#endif
        }

        private void miSave_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrWhiteSpace(heightmapFilePath))
            {
                var sfd = new SaveFileDialog
                {
                    Filter = "16-bit raw heightmap (*.r16)|*.r16",
                    Title = "Save heightmap file"
                };
                if (sfd.ShowDialog() == true)
                {
                    heightmapFilePath = sfd.FileName;
                }
            }
            if (!string.IsNullOrWhiteSpace(heightmapFilePath))
            {
                EditorPlatform.QueueSaveHeightmap(heightmapFilePath);
            }
        }

        private void OnTransactionPublished(EditorCommandList commands)
        {
            foreach (var entry in commands)
            {
                if (entry.Type == EditorCommandType.AddMaterial)
                {
                    ref readonly AddMaterialCommand cmd = ref entry.As<AddMaterialCommand>();
                    uint materialId = cmd.MaterialId;

                    var materialVm = App.Current.Document.TerrainMaterials.FirstOrDefault(
                        vm => vm.MaterialId == materialId);
                    if (materialVm != null)
                    {
                        cvsTerrainMaterials.View.MoveCurrentTo(materialVm);
                    }
                }
                else if (entry.Type == EditorCommandType.AddObject)
                {
                    ref readonly AddObjectCommand cmd = ref entry.As<AddObjectCommand>();
                    ref EditorUiState state = ref EditorCore.GetUiState();
                    state.SelectedObjectCount = 1;
                    state.SelectedObjectIds[0] = cmd.ObjectId;
                }
            }
        }

        private void OnPerfCountersUpdated(CollatedPerfCounters perfCounters)
        {
            // try calculate the line length based on the current width of the text block
            int lineLength = 51;
            double availableWidth = (tbPerfCounters.Parent as FrameworkElement).ActualWidth;
            if (availableWidth > 0)
            {
                // this is the character advance width of Consolas (which is a monospace font)
                const double advanceWidth = 0.5498046875;
                double advanceWidthForFontSize = advanceWidth * tbPerfCounters.FontSize;
                lineLength = (int)Math.Floor(availableWidth / advanceWidthForFontSize);
            }
            if (lineLength < 25)
            {
                lineLength = 25;
            }

            const int indentWidth = 2;
            int counterNameLength = lineLength - 17;
            const double minMsThreshold = 0.01;

            var perfCounterSummaryBuilder = new StringBuilder();

            double frameMs = perfCounters.FrameTime.TotalMilliseconds;
            int fps = (int)(1000.0 / perfCounters.FrameTime.TotalMilliseconds);
            perfCounterSummaryBuilder.AppendLine(
                $"{"Total Frame Time".PadRight(counterNameLength).Substring(0, counterNameLength)}{frameMs,7:#0.00}ms{fps,5:##0}fps");
            perfCounterSummaryBuilder.AppendLine(new string('-', lineLength));

            void PrintCounter(CollatedPerfCounter counter, int indent)
            {
                int length = counterNameLength - (indent * indentWidth);
                string padding = new string(' ', indent * indentWidth);
                string counterName = counter.Name.PadRight(length).Substring(0, length);
                double ms = counter.Elapsed.TotalMilliseconds;
                if (ms >= minMsThreshold)
                {
                    double pct = (counter.Elapsed.Ticks * 100.0) / perfCounters.FrameTime.Ticks;
                    perfCounterSummaryBuilder.AppendLine($"{padding}{counterName}{ms,7:#0.00}ms{pct,7:#0.0}%");
                }

                foreach (var childCounter in counter.Children)
                {
                    PrintCounter(childCounter, indent + 1);
                }
            }

            foreach (var counter in perfCounters.RootCounter.Children)
            {
                PrintCounter(counter, 0);
            }

            tbPerfCounters.Text = perfCounterSummaryBuilder.ToString();
        }
    }
}