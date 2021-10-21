using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Data;
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

        private void OnPerfCountersUpdated(EditorPerformanceCounters perfCounters)
        {
            const int lineLength = 51;
            int counterNameLength = lineLength - 17;

            var perfCounterSummaryBuilder = new StringBuilder();

            double frameMs = perfCounters.FrameTime.TotalMilliseconds;
            int fps = (int)(1000.0 / perfCounters.FrameTime.TotalMilliseconds);
            perfCounterSummaryBuilder.AppendLine(
                $"{"Total Frame Time".PadRight(counterNameLength).Substring(0, counterNameLength)}{frameMs,7:#0.00}ms{fps,5:##0}fps");
            perfCounterSummaryBuilder.AppendLine(new string('-', lineLength));

            foreach (var counter in perfCounters.Counters)
            {
                string counterName = counter.Key.PadRight(counterNameLength).Substring(0, counterNameLength);
                double ms = counter.Value.TotalMilliseconds;
                double pct = (counter.Value.Ticks * 100.0) / perfCounters.FrameTime.Ticks;
                perfCounterSummaryBuilder.AppendLine($"{counterName}{ms,7:#0.00}ms{pct,7:#0.0}%");
            }

            tbPerfCounters.Text = perfCounterSummaryBuilder.ToString();
        }
    }
}