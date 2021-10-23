using System;
using System.Threading;
using System.Windows;
using System.Windows.Threading;
using Sierra.Core;
using Sierra.Platform;
using Sierra.Utilities.Binding;
using Sierra.ViewModels;

namespace Sierra
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private DispatcherTimer updateUiTimer;

        public EditorAssetsViewModel Assets { get; private set; }
        public EditorDocumentViewModel Document { get; private set; }

        public new static App Current => Application.Current as App;

        internal delegate void PerformanceCountersUpdatedEventHandler(CollatedPerfCounters perfCounters);
        internal static event PerformanceCountersUpdatedEventHandler PerformanceCountersUpdated;

        protected override void OnStartup(StartupEventArgs e)
        {
            Assets = (EditorAssetsViewModel)FindResource("EditorAssets");
            Document = (EditorDocumentViewModel)FindResource("EditorDocument");

            EditorCore.AssetRegistered += Assets.OnAssetRegistered;
            EditorCore.TransactionPublished += Document.OnTransactionPublished;

            updateUiTimer = new DispatcherTimer(DispatcherPriority.Send)
            {
                Interval = TimeSpan.FromMilliseconds(100)
            };
            updateUiTimer.Tick += UpdateUiTick;
            updateUiTimer.Start();

            var coreThread = new Thread(() => EditorPlatform.Tick());
            coreThread.Start();

            base.OnStartup(e);
        }

        protected override void OnExit(ExitEventArgs e)
        {
            updateUiTimer.Stop();
            EditorPlatform.Shutdown();

            base.OnExit(e);
        }

        private void UpdateUiTick(object sender, EventArgs e)
        {
            ref EditorUiState uiState = ref EditorCore.GetUiState();
            EditorBindingEngine.UpdateBindings(ref uiState);
            EditorCommands.Update(Document, ref uiState);

            var collatedPerfCounters = PerfCounters.Collate();
            PerformanceCountersUpdated?.Invoke(collatedPerfCounters);
        }
    }
}