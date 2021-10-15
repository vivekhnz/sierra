using System;
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
        private const int PerfCounterHistoryFrameCount = 30;
        private const int TicksBetweenUiUpdates = 4;

        private EditorPerformanceCounters[] perfCountersByFrame;
        private int currentPerfCounterIndex;
        private EditorPerformanceCounters averagedPerfCounters;
        private int ticksUntilUiUpdate;

        private DateTime lastFrameTime;
        private DispatcherTimer renderTimer;

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

            EditorPlatform.Initialize();

            perfCountersByFrame = new EditorPerformanceCounters[PerfCounterHistoryFrameCount];
            for (int i = 0; i < PerfCounterHistoryFrameCount; i++)
            {
                perfCountersByFrame[i] = new EditorPerformanceCounters();
            }
            averagedPerfCounters = new EditorPerformanceCounters();
            currentPerfCounterIndex = 0;
            ticksUntilUiUpdate = TicksBetweenUiUpdates;

            renderTimer = new DispatcherTimer(DispatcherPriority.Send)
            {
                Interval = TimeSpan.FromMilliseconds(0.5)
            };
            renderTimer.Tick += OnTick;
            renderTimer.Start();

            base.OnStartup(e);
        }

        protected override void OnExit(ExitEventArgs e)
        {
            renderTimer.Stop();
            EditorPlatform.Shutdown();

            base.OnExit(e);
        }

        private void OnTick(object sender, EventArgs e)
        {
            EditorPerformanceCounters thisFramePerfCounters = perfCountersByFrame[currentPerfCounterIndex];
            currentPerfCounterIndex++;
            if (currentPerfCounterIndex == PerfCounterHistoryFrameCount)
            {
                currentPerfCounterIndex = 0;
            }
            thisFramePerfCounters.Reset();

            DateTime now = DateTime.UtcNow;
            thisFramePerfCounters.FrameTime = now - lastFrameTime;
            lastFrameTime = now;

            EditorPlatform.Tick(thisFramePerfCounters);

            ticksUntilUiUpdate--;
            if (ticksUntilUiUpdate < 0)
            {
                ref EditorUiState uiState = ref EditorCore.GetUiState();
                using (thisFramePerfCounters.Measure_UpdateBindings())
                {
                    EditorBindingEngine.UpdateBindings(ref uiState);
                }
                EditorCommands.Update(Document, ref uiState);

                averagedPerfCounters.Reset();
                foreach (var framePerfCounters in perfCountersByFrame)
                {
                    averagedPerfCounters.FrameTime += framePerfCounters.FrameTime;
                    averagedPerfCounters.GetInputState += framePerfCounters.GetInputState;
                    averagedPerfCounters.CoreUpdate += framePerfCounters.CoreUpdate;
                    averagedPerfCounters.RenderViewports += framePerfCounters.RenderViewports;
                    averagedPerfCounters.RenderSceneView += framePerfCounters.RenderSceneView;
                    averagedPerfCounters.UpdateBindings += framePerfCounters.UpdateBindings;
                }
                averagedPerfCounters.FrameTime /= (double)PerfCounterHistoryFrameCount;
                averagedPerfCounters.GetInputState /= (double)PerfCounterHistoryFrameCount;
                averagedPerfCounters.CoreUpdate /= (double)PerfCounterHistoryFrameCount;
                averagedPerfCounters.RenderViewports /= (double)PerfCounterHistoryFrameCount;
                averagedPerfCounters.RenderSceneView /= (double)PerfCounterHistoryFrameCount;
                averagedPerfCounters.UpdateBindings /= (double)PerfCounterHistoryFrameCount;

                PerformanceCountersUpdated?.Invoke(averagedPerfCounters);

                ticksUntilUiUpdate = TicksBetweenUiUpdates;
            }
        }
    }
}
