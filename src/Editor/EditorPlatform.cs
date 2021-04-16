using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Threading;
using Terrain.Engine.Interop;

namespace Terrain.Editor
{
    internal static class EditorPlatform
    {
        private static DispatcherTimer renderTimer;
        private static DateTime lastTickTime;
        private static Win32.WndProc defWndProc = new Win32.WndProc(Win32.DefWindowProc);

        internal static void Initialize()
        {
            uint editorMemorySizeInBytes = 32 * 1024 * 1024;
            uint engineMemorySizeInBytes = 480 * 1024 * 1024;
            uint appMemorySizeInBytes = editorMemorySizeInBytes + engineMemorySizeInBytes;
            IntPtr appMemoryPtr = Win32.VirtualAlloc(IntPtr.Zero, appMemorySizeInBytes,
                Win32.AllocationType.Reserve | Win32.AllocationType.Commit,
                Win32.MemoryProtection.ReadWrite);

            var interopHelper = new WindowInteropHelper(Application.Current.MainWindow);
            IntPtr mainWindowHwnd = interopHelper.EnsureHandle();

            IntPtr appInstance = Win32.GetModuleHandle(null);
            Win32.WindowClass dummyWindowClass = new Win32.WindowClass
            {
                lpfnWndProc = Marshal.GetFunctionPointerForDelegate(defWndProc),
                hInstance = appInstance,
                lpszClassName = "TerrainOpenGLDummyWindowClass"
            };
            Win32.RegisterClass(ref dummyWindowClass);
            IntPtr dummyWindowHwnd = Win32.CreateWindowEx(0, dummyWindowClass.lpszClassName,
                "TerrainOpenGLDummyWindow", 0, 0, 0, 100, 100, IntPtr.Zero, IntPtr.Zero, appInstance,
                IntPtr.Zero);

            var initParams = new EditorInitPlatformParamsProxy
            {
                memoryPtr = appMemoryPtr,
                editorMemorySize = editorMemorySizeInBytes,
                engineMemorySize = engineMemorySizeInBytes,

                editorCodeDllPath = Path.Combine(
                    AppDomain.CurrentDomain.BaseDirectory, "terrain_editor.dll"),
                editorCodeDllShadowCopyPath = Path.Combine(
                    AppDomain.CurrentDomain.BaseDirectory, "terrain_editor.copy.dll"),
                editorCodeBuildLockFilePath = Path.Combine(
                    AppDomain.CurrentDomain.BaseDirectory, "build.lock"),

                instance = appInstance,
                mainWindowHwnd = mainWindowHwnd,
                dummyWindowHwnd = dummyWindowHwnd
            };
            EngineInterop.InitializeEngine(initParams);

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
