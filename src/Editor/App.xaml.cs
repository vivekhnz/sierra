using System;
using System.Linq;
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
        private EditorPerformanceCounters averagedPerfCounters;
        private DispatcherTimer updateUiTimer;

        public EditorAssetsViewModel Assets { get; private set; }
        public EditorDocumentViewModel Document { get; private set; }

        public new static App Current => Application.Current as App;

        internal delegate void PerformanceCountersUpdatedEventHandler(EditorPerformanceCounters perfCounters);
        internal static event PerformanceCountersUpdatedEventHandler PerformanceCountersUpdated;

        protected override void OnStartup(StartupEventArgs e)
        {
            Assets = (EditorAssetsViewModel)FindResource("EditorAssets");
            Document = (EditorDocumentViewModel)FindResource("EditorDocument");

            EditorCore.AssetRegistered += Assets.OnAssetRegistered;
            EditorCore.TransactionPublished += Document.OnTransactionPublished;

            averagedPerfCounters = new EditorPerformanceCounters();
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

            averagedPerfCounters.Reset();
            int frameCount = 0;
            foreach (var framePerfCounters in EditorPlatform.PerfCountersByFrame)
            {
                try
                {
                    foreach (var counter in framePerfCounters.Counters)
                    {
                        averagedPerfCounters.AddToCounter(counter.Key, counter.Value);
                    }
                    averagedPerfCounters.FrameTime += framePerfCounters.FrameTime;
                    frameCount++;
                }
                catch (InvalidOperationException)
                {
                    // Typically this exception occurs when the counter collection is modified by the core
                    // tick thread. We can ignore it.
                }
            }
            averagedPerfCounters.FrameTime /= (double)frameCount;
            for (int i = 0; i < averagedPerfCounters.Counters.Count; i++)
            {
                var counter = averagedPerfCounters.Counters.ElementAt(i);
                averagedPerfCounters.Counters[counter.Key] /= (double)frameCount;
            }

            PerformanceCountersUpdated?.Invoke(averagedPerfCounters);
        }
    }
}