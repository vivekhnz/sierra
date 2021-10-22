using System;
using System.Collections.Generic;
using System.Diagnostics;
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

            var averagedPerfCounters = new EditorPerformanceCounters();
            var counterStack = new Stack<(EditorPerformanceCounter, DateTime)>();
            var counterHistory = new Dictionary<EditorPerformanceCounter, List<TimeSpan>>();
            int frameCount = 0;
            foreach (var frameLog in EditorPlatform.PerfCounterLogsByFrame)
            {
                if (!frameLog.IsCompleted) continue;

                try
                {
                    var parentCounter = averagedPerfCounters.RootCounter;
                    var thisFrameCounters = new Dictionary<EditorPerformanceCounter, TimeSpan>();
                    counterStack.Clear();
                    foreach (var entry in frameLog.Entries)
                    {
                        if (entry.IsEnd)
                        {
                            var (counter, startDt) = counterStack.Pop();
                            Debug.Assert(entry.CounterName == counter.Name);
                            TimeSpan elapsed = entry.LogDt - startDt;

                            if (thisFrameCounters.TryGetValue(counter, out var elapsedThisFrame))
                            {
                                thisFrameCounters[counter] = elapsedThisFrame + elapsed;
                            }
                            else
                            {
                                thisFrameCounters.Add(counter, elapsed);
                            }

                            if (counterStack.TryPeek(out var topOfStack))
                            {
                                (parentCounter, _) = topOfStack;
                            }
                            else
                            {
                                parentCounter = averagedPerfCounters.RootCounter;
                            }
                        }
                        else
                        {
                            var counter = parentCounter.Children.FirstOrDefault(c => c.Name == entry.CounterName);
                            if (counter == null)
                            {
                                counter = new EditorPerformanceCounter
                                {
                                    Name = entry.CounterName,
                                    Elapsed = TimeSpan.Zero
                                };
                                parentCounter.Children.Add(counter);
                            }
                            counterStack.Push((counter, entry.LogDt));
                            parentCounter = counter;
                        }
                    }
                    Debug.Assert(counterStack.Count == 0);

                    foreach (var counter in thisFrameCounters)
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
                foreach (var kvp in counterHistory)
                {
                    var counter = kvp.Key;
                    var history = kvp.Value;

                    TimeSpan sum = TimeSpan.Zero;
                    foreach (var elapsed in history)
                    {
                        sum += elapsed;
                    }
                    counter.Elapsed = sum / history.Count;
                }
            }

            PerformanceCountersUpdated?.Invoke(averagedPerfCounters);
        }
    }
}