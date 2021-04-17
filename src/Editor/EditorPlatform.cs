using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Threading;
using Terrain.Engine.Interop;

namespace Terrain.Editor
{
    internal struct EditorViewportWindow
    {
        public IntPtr Hwnd;
        public IntPtr WindowPtr;
    }

    internal static class EditorPlatform
    {
        private static readonly string ViewportWindowClassName = "TerrainOpenGLViewportWindowClass";

        private static IntPtr appInstance = Win32.GetModuleHandle(null);
        private static Win32.WndProc defWndProc = new Win32.WndProc(Win32.DefWindowProc);
        private static Win32.WndProc viewportWndProc = new Win32.WndProc(ViewportWindowProc);

        private static DispatcherTimer renderTimer;
        private static DateTime lastTickTime;

        private static bool shouldCaptureMouse;
        private static bool wasMouseCaptured;
        private static float nextMouseScrollOffsetY;

        private delegate void PlatformCaptureMouse();
        private static PlatformCaptureMouse EditorPlatformCaptureMouse = () =>
        {
            shouldCaptureMouse = true;
        };

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

            var dummyWindowClass = new Win32.WindowClass
            {
                lpfnWndProc = Marshal.GetFunctionPointerForDelegate(defWndProc),
                hInstance = appInstance,
                lpszClassName = "TerrainOpenGLDummyWindowClass"
            };
            Win32.RegisterClass(ref dummyWindowClass);
            IntPtr dummyWindowHwnd = Win32.CreateWindowEx(0, dummyWindowClass.lpszClassName,
                "TerrainOpenGLDummyWindow", 0, 0, 0, 100, 100, IntPtr.Zero, IntPtr.Zero, appInstance,
                IntPtr.Zero);

            IntPtr dummyDeviceContext = Win32.GetDC(dummyWindowHwnd);
            ConfigureDeviceContextForOpenGL(dummyDeviceContext);
            IntPtr glRenderingContext = Win32.CreateGLContext(dummyDeviceContext);
            Win32.MakeGLContextCurrent(dummyDeviceContext, glRenderingContext);

            var viewportWindowClass = new Win32.WindowClass
            {
                style = Win32.WindowClassStyles.HorizontalRedraw |
                    Win32.WindowClassStyles.VerticalRedraw |
                    Win32.WindowClassStyles.OwnDC,
                lpfnWndProc = Marshal.GetFunctionPointerForDelegate(viewportWndProc),
                hInstance = appInstance,
                lpszClassName = ViewportWindowClassName
            };
            Win32.RegisterClass(ref viewportWindowClass);

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
                dummyWindowHwnd = dummyWindowHwnd,
                glRenderingContext = glRenderingContext,

                platformCaptureMouse = Marshal.GetFunctionPointerForDelegate(
                    EditorPlatformCaptureMouse)
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

        private static void ConfigureDeviceContextForOpenGL(IntPtr deviceContext)
        {
            var pfd = new Win32.PixelFormatDescriptor
            {
                Size = (ushort)Marshal.SizeOf<Win32.PixelFormatDescriptor>(),
                Version = 1,
                Flags = Win32.PixelFormatDescriptorFlags.DrawToWindow |
                    Win32.PixelFormatDescriptorFlags.SupportOpenGL |
                    Win32.PixelFormatDescriptorFlags.DoubleBuffer,
                PixelType = Win32.PixelFormatDescriptorPixelType.RGBA,
                ColorBits = 32,
                DepthBits = 16,
                LayerType = Win32.PixelFormatDescriptorLayerType.MainPlane
            };
            int pixelFormat = Win32.ChoosePixelFormat(deviceContext, ref pfd);
            Win32.SetPixelFormat(deviceContext, pixelFormat, ref pfd);
        }

        private static IntPtr ViewportWindowProc(
            IntPtr hwnd, Win32.WindowMessage message, IntPtr wParam, IntPtr lParam)
        {
            IntPtr resultHandled = new IntPtr(1);
            switch (message)
            {
                case Win32.WindowMessage.SetCursor:
                    bool hideCursor = shouldCaptureMouse || wasMouseCaptured;
                    Win32.SetCursor(hideCursor
                        ? IntPtr.Zero
                        : Win32.LoadCursor(IntPtr.Zero, Win32.Cursor.Arrow));
                    return resultHandled;

                case Win32.WindowMessage.MouseWheel:
                    short delta = (short)(wParam.ToInt64() >> 16);
                    short direction = (short)(lParam.ToInt64() >> 16);
                    nextMouseScrollOffsetY += delta * direction;
                    return resultHandled;
            }
            return Win32.DefWindowProc(hwnd, message, wParam, lParam);
        }

        private static void OnTick(object sender, EventArgs e)
        {
            DateTime now = DateTime.UtcNow;
            float deltaTime = (float)((now - lastTickTime).TotalSeconds);
            lastTickTime = now;

            var tickParams = new EditorTickAppParamsProxy
            {
                shouldCaptureMouse = shouldCaptureMouse,
                wasMouseCaptured = wasMouseCaptured,
                nextMouseScrollOffsetY = nextMouseScrollOffsetY
            };
            wasMouseCaptured = shouldCaptureMouse;
            shouldCaptureMouse = false;
            nextMouseScrollOffsetY = 0;

            EngineInterop.TickApp(deltaTime, tickParams);
        }

        internal static void Shutdown()
        {
            renderTimer.Stop();
            EngineInterop.Shutdown();
        }

        internal static EditorViewportWindow CreateViewportWindow(IntPtr parentHwnd,
            uint x, uint y, uint width, uint height, EditorView view)
        {
            IntPtr hwnd = Win32.CreateWindowEx(0, ViewportWindowClassName,
                "TerrainOpenGLViewportWindow",
                Win32.WindowStyles.Child | Win32.WindowStyles.Visible, 0, 0, 1, 1, parentHwnd,
                IntPtr.Zero, appInstance, IntPtr.Zero);
            IntPtr deviceContext = Win32.GetDC(hwnd);
            ConfigureDeviceContextForOpenGL(deviceContext);

            IntPtr windowPtr = EngineInterop.CreateViewportWindow(
                deviceContext, x, y, width, height, view);

            return new EditorViewportWindow
            {
                Hwnd = hwnd,
                WindowPtr = windowPtr
            };
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
