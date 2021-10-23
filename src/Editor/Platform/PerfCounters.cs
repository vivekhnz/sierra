using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace Sierra.Platform
{
    internal static class PerfCounters
    {
        [DebuggerDisplay("{DebuggerDisplay,nq}")]
        private class PerfCounterLogEntry
        {
            private string DebuggerDisplay => $"[{LogDt:hh:mm:ss.fff}] {(IsEnd ? "END" : "START")} {CounterName}";

            public string CounterName;
            public DateTime LogDt;
            public bool IsEnd;
        }
        private class PerfCounterLog
        {
            public TimeSpan FrameTime;
            public List<PerfCounterLogEntry> Entries = new List<PerfCounterLogEntry>();
            public bool IsOpen;
        }
        internal class TimedBlock : IDisposable
        {
            private readonly string name;

            public TimedBlock(string name)
            {
                PerfCounters.StartPerfCounter(name);
                this.name = name;
            }

            public void Dispose()
            {
                PerfCounters.EndPerfCounter(name);
            }
        }

        const int PerfCounterHistoryFrameCount = 30;

        private static PerfCounterLog[] frameLogs;
        private static PerfCounterLog currentLog;
        private static int currentLogIndex;

        internal static void StartFrame(TimeSpan elapsedFrameTime)
        {
            if (frameLogs == null)
            {
                frameLogs = new PerfCounterLog[PerfCounterHistoryFrameCount];
                for (int i = 0; i < PerfCounterHistoryFrameCount; i++)
                {
                    frameLogs[i] = new PerfCounterLog();
                }
            }

            currentLog = frameLogs[currentLogIndex];
            currentLogIndex++;
            if (currentLogIndex == PerfCounterHistoryFrameCount)
            {
                currentLogIndex = 0;
            }

            currentLog.FrameTime = elapsedFrameTime;
            currentLog.Entries.Clear();
            currentLog.IsOpen = true;
        }

        internal static void EndFrame()
        {
            currentLog.IsOpen = false;
        }

        private static void AddPerfCounterEntry(PerfCounterLogEntry entry)
        {
            Debug.Assert(currentLog.IsOpen);
            currentLog.Entries.Add(entry);
        }

        internal static void StartPerfCounter(string counterName)
        {
            AddPerfCounterEntry(new PerfCounterLogEntry
            {
                CounterName = counterName,
                LogDt = DateTime.UtcNow,
                IsEnd = false
            });
        }

        internal static void EndPerfCounter(string counterName)
        {
            AddPerfCounterEntry(new PerfCounterLogEntry
            {
                CounterName = counterName,
                LogDt = DateTime.UtcNow,
                IsEnd = true
            });
        }

        internal static TimedBlock Measure(string counterName)
        {
            return new TimedBlock(counterName);
        }

        internal static CollatedPerfCounters Collate()
        {
            var result = new CollatedPerfCounters();

            var counterStack = new Stack<(CollatedPerfCounter, DateTime)>();
            var counterHistory = new Dictionary<CollatedPerfCounter, List<TimeSpan>>();
            int frameCount = 0;
            foreach (var frameLog in frameLogs)
            {
                if (frameLog.IsOpen) continue;

                try
                {
                    var parentCounter = result.RootCounter;
                    var thisFrameCounters = new Dictionary<CollatedPerfCounter, TimeSpan>();
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
                                parentCounter = result.RootCounter;
                            }
                        }
                        else
                        {
                            var counter = parentCounter.Children.FirstOrDefault(c => c.Name == entry.CounterName);
                            if (counter == null)
                            {
                                counter = new CollatedPerfCounter
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
                    result.FrameTime += frameLog.FrameTime;
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
                result.FrameTime /= frameCount;
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

            return result;
        }
    }

    internal class CollatedPerfCounter
    {
        public string Name;
        public TimeSpan Elapsed;
        public List<CollatedPerfCounter> Children = new List<CollatedPerfCounter>();
    }
    internal class CollatedPerfCounters
    {
        public TimeSpan FrameTime;
        public CollatedPerfCounter RootCounter = new CollatedPerfCounter();
    }
}