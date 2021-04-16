using System;
using System.Windows.Threading;
using Terrain.Engine.Interop;

namespace Terrain.Editor
{
    internal static class EditorPlatform
    {
        private static DispatcherTimer renderTimer;
        private static DateTime lastTickTime;

        internal static void Initialize()
        {
            uint editorMemorySizeInBytes = 32 * 1024 * 1024;
            uint engineMemorySizeInBytes = 480 * 1024 * 1024;
            uint appMemorySizeInBytes = editorMemorySizeInBytes + engineMemorySizeInBytes;
            IntPtr appMemoryPtr = Win32.VirtualAlloc(IntPtr.Zero, appMemorySizeInBytes,
                Win32.AllocationType.Reserve | Win32.AllocationType.Commit,
                Win32.MemoryProtection.ReadWrite);

            EngineInterop.InitializeEngine(appMemoryPtr, editorMemorySizeInBytes, engineMemorySizeInBytes);

            lastTickTime = DateTime.UtcNow;
            renderTimer = new DispatcherTimer(DispatcherPriority.Send)
            {
                Interval = TimeSpan.FromMilliseconds(1)
            };
            renderTimer.Tick += OnTick;
            renderTimer.Start();
        }

        private static void OnTick(object sender, EventArgs e)
        {
            DateTime now = DateTime.UtcNow;
            float deltaTime = (float)((now - lastTickTime).TotalSeconds);
            lastTickTime = now;

            EngineInterop.TickApp(deltaTime);
        }

        internal static void Shutdown()
        {
            renderTimer.Stop();
            EngineInterop.Shutdown();
        }

        internal static EditorPlatformViewportWindow CreateViewportWindow(IntPtr parentHwnd,
            uint x, uint y, uint width, uint height, EditorView view)
        {
            return EngineInterop.CreateViewportWindow(parentHwnd, x, y, width, height, view);
        }

        internal static void DestroyViewportWindow(IntPtr hwnd)
        {
            Win32.DestroyWindow(hwnd);
        }

        internal static void ResizeViewportWindow(IntPtr windowPtr,
            uint x, uint y, uint width, uint height)
        {
            EngineInterop.ResizeViewportWindow(windowPtr, x, y, width, height);
        }
    }
}
