using System;
using System.Collections.Generic;
using System.Diagnostics;
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
            var logStack = new Stack<(string, DateTime)>();
            var counterHistory = new Dictionary<string, List<TimeSpan>>();
            int frameCount = 0;
            foreach (var frameLog in EditorPlatform.PerfCounterLogsByFrame)
            {
                if (!frameLog.IsCompleted) continue;

                try
                {
                    var counters = new Dictionary<string, TimeSpan>();
                    logStack.Clear();
                    foreach (var entry in frameLog.Entries)
                    {
                        if (entry.IsEnd)
                        {
                            var (startName, startDt) = logStack.Pop();
                            Debug.Assert(entry.CounterName == startName);
                            var elapsed = entry.LogDt - startDt;

                            if (counters.TryGetValue(entry.CounterName, out var alreadyElapsed))
                            {
                                counters[entry.CounterName] = alreadyElapsed + elapsed;
                            }
                            else
                            {
                                counters.Add(entry.CounterName, elapsed);
                            }
                        }
                        else
                        {
                            logStack.Push((entry.CounterName, entry.LogDt));
                        }
                    }
                    Debug.Assert(logStack.Count == 0);

                    foreach (var counter in counters)
                    {
                        List<TimeSpan> history;
                        if (!counterHistory.TryGetValue(counter.Key, out history))
                        {
                            history = new List<TimeSpan>();
                            counterHistory.Add(counter.Key, history);
                        }
                        history.Add(counter.Value);
                    }
                    averagedPerfCounters.FrameTime += frameLog.FrameTime;
                    frameCount++;
                }
                catch (InvalidOperationException)
                {
                    // Typically this exception occurs when the counter collection is modified by the core
                    // tick thread. We can ignore it.
                }
            }
            if (frameCount > 0)
            {
                averagedPerfCounters.FrameTime /= frameCount;
                foreach (var counter in counterHistory)
                {
                    var avg = TimeSpan.Zero;
                    foreach (var elapsed in counter.Value)
                    {
                        avg += elapsed;
                    }
                    avg /= counter.Value.Count;

                    averagedPerfCounters.Counters.Add(counter.Key, avg);
                }
            }

            PerformanceCountersUpdated?.Invoke(averagedPerfCounters);
        }
    }
}