using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Threading;
using Terrain.Engine.Interop;

namespace Terrain.Editor
{
    public enum EditorView
    {
        None = 0,
        Scene = 1,
        HeightmapPreview = 2
    };

    [Flags]
    internal enum EditorInputButtons : ulong
    {
        MouseLeft = 1L << 0,
        MouseMiddle = 1L << 1,
        MouseRight = 1L << 2,
        KeySpace = 1L << 3,
        Key0 = 1L << 4,
        Key1 = 1L << 5,
        Key2 = 1L << 6,
        Key3 = 1L << 7,
        Key4 = 1L << 8,
        Key5 = 1L << 9,
        Key6 = 1L << 10,
        Key7 = 1L << 11,
        Key8 = 1L << 12,
        Key9 = 1L << 13,
        KeyA = 1L << 14,
        KeyB = 1L << 15,
        KeyC = 1L << 16,
        KeyD = 1L << 17,
        KeyE = 1L << 18,
        KeyF = 1L << 19,
        KeyG = 1L << 20,
        KeyH = 1L << 21,
        KeyI = 1L << 22,
        KeyJ = 1L << 23,
        KeyK = 1L << 24,
        KeyL = 1L << 25,
        KeyM = 1L << 26,
        KeyN = 1L << 27,
        KeyO = 1L << 28,
        KeyP = 1L << 29,
        KeyQ = 1L << 30,
        KeyR = 1L << 31,
        KeyS = 1L << 32,
        KeyT = 1L << 33,
        KeyU = 1L << 34,
        KeyV = 1L << 35,
        KeyW = 1L << 36,
        KeyX = 1L << 37,
        KeyY = 1L << 38,
        KeyZ = 1L << 39,
        KeyEscape = 1L << 40,
        KeyEnter = 1L << 41,
        KeyRight = 1L << 42,
        KeyLeft = 1L << 43,
        KeyDown = 1L << 44,
        KeyUp = 1L << 45,
        KeyF1 = 1L << 46,
        KeyF2 = 1L << 47,
        KeyF3 = 1L << 48,
        KeyF4 = 1L << 49,
        KeyF5 = 1L << 50,
        KeyF6 = 1L << 51,
        KeyF7 = 1L << 52,
        KeyF8 = 1L << 53,
        KeyF9 = 1L << 54,
        KeyF10 = 1L << 55,
        KeyF11 = 1L << 56,
        KeyF12 = 1L << 57,
        KeyLeftShift = 1L << 58,
        KeyLeftControl = 1L << 59,
        KeyRightShift = 1L << 60,
        KeyRightControl = 1L << 61,
        KeyAlt = 1L << 62
    }

    internal class EditorViewportWindow
    {
        public IntPtr Hwnd;
        public IntPtr DeviceContext;
        public EditorView View;
        public EditorViewContextProxy ViewContext;
    }

    internal static class EditorPlatform
    {
        private class AssetLoadRequest
        {
            public uint AssetId;
            public string Path;
        }
        private class WatchedAsset
        {
            public uint AssetId;
            public string Path;
            public DateTime LastUpdatedTimeUtc;
        }
        private class ReloadableCode
        {
            public string DllPath;
            public string DllShadowCopyPath;
            public DateTime DllLastWriteTimeUtc;
        }

        private static readonly string ViewportWindowClassName = "TerrainOpenGLViewportWindowClass";

        private static IntPtr appInstance = Win32.GetModuleHandle(null);
        private static Win32.WndProc defWndProc = new Win32.WndProc(Win32.DefWindowProc);
        private static Win32.WndProc viewportWndProc = new Win32.WndProc(ViewportWindowProc);

        private static string buildLockFilePath;
        private static ReloadableCode engineCode;
        private static ReloadableCode editorCode;

        private static IntPtr mainWindowHwnd;
        private static IntPtr dummyWindowHwnd;
        private static IntPtr glRenderingContext;

        private static DispatcherTimer renderTimer;
        private static DateTime lastTickTime;

        private static Point prevMousePosWindowSpace;
        private static bool shouldCaptureMouse;
        private static bool wasMouseCaptured;
        private static Point capturedMousePosWindowSpace;
        private static float nextMouseScrollOffsetY;
        private static bool isMouseLeftDown;
        private static bool isMouseMiddleDown;
        private static bool isMouseRightDown;
        private static EditorInputButtons prevPressedButtons;

        private static List<EditorViewportWindow> viewportWindows = new List<EditorViewportWindow>();

        private static string assetsDirectoryPath;
        private static List<AssetLoadRequest> assetLoadRequests = new List<AssetLoadRequest>();
        private static List<WatchedAsset> watchedAssets = new List<WatchedAsset>();

        private delegate void PlatformCaptureMouse();
        private delegate void PlatformLogMessage(string message);
        private delegate bool PlatformQueueAssetLoad(uint assetId, string relativePath);
        private delegate void PlatformWatchAssetFile(uint assetId, string relativePath);

        private static PlatformCaptureMouse editorPlatformCaptureMouse = CaptureMouse;
        private static PlatformLogMessage editorPlatformLogMessage = LogMessage;
        private static PlatformQueueAssetLoad editorPlatformQueueAssetLoad = QueueAssetLoadRelative;
        private static PlatformWatchAssetFile editorPlatformWatchAssetFile = WatchAssetFile;

        private static IntPtr engineMemoryPtr;

        internal static Engine Engine { get; private set; }

        internal static void Initialize()
        {
            assetsDirectoryPath = Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory, "../../../data");
            buildLockFilePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "build.lock");
            engineCode = new ReloadableCode
            {
                DllPath = Path.Combine(
                    AppDomain.CurrentDomain.BaseDirectory, "terrain_engine.dll"),
                DllShadowCopyPath = Path.Combine(
                    AppDomain.CurrentDomain.BaseDirectory, "terrain_engine.copy.dll")
            };
            editorCode = new ReloadableCode
            {
                DllPath = Path.Combine(
                    AppDomain.CurrentDomain.BaseDirectory, "terrain_editor.dll"),
                DllShadowCopyPath = Path.Combine(
                    AppDomain.CurrentDomain.BaseDirectory, "terrain_editor.copy.dll")
            };

            uint editorMemorySizeInBytes = 32 * 1024 * 1024;
            uint engineMemorySizeInBytes = 480 * 1024 * 1024;
            uint appMemorySizeInBytes = editorMemorySizeInBytes + engineMemorySizeInBytes;
            IntPtr appMemoryPtr = Win32.VirtualAlloc(IntPtr.Zero, appMemorySizeInBytes,
                Win32.AllocationType.Reserve | Win32.AllocationType.Commit,
                Win32.MemoryProtection.ReadWrite);

            var interopHelper = new WindowInteropHelper(Application.Current.MainWindow);
            mainWindowHwnd = interopHelper.EnsureHandle();

            var dummyWindowClass = new Win32.WindowClass
            {
                lpfnWndProc = Marshal.GetFunctionPointerForDelegate(defWndProc),
                hInstance = appInstance,
                lpszClassName = "TerrainOpenGLDummyWindowClass"
            };
            Win32.RegisterClass(ref dummyWindowClass);
            dummyWindowHwnd = Win32.CreateWindowEx(0, dummyWindowClass.lpszClassName,
                "TerrainOpenGLDummyWindow", 0, 0, 0, 100, 100, IntPtr.Zero, IntPtr.Zero, appInstance,
                IntPtr.Zero);

            IntPtr dummyDeviceContext = Win32.GetDC(dummyWindowHwnd);
            ConfigureDeviceContextForOpenGL(dummyDeviceContext);
            glRenderingContext = Win32.CreateGLContext(dummyDeviceContext);
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

                platformCaptureMouse = Marshal.GetFunctionPointerForDelegate(editorPlatformCaptureMouse),
                platformLogMessage = Marshal.GetFunctionPointerForDelegate(editorPlatformLogMessage),
                platformQueueAssetLoad = Marshal.GetFunctionPointerForDelegate(editorPlatformQueueAssetLoad),
                platformWatchAssetFile = Marshal.GetFunctionPointerForDelegate(editorPlatformWatchAssetFile)
            };
            EngineInterop.InitializeEngine(initParams);

            engineMemoryPtr = EngineInterop.GetEngineMemory();
            IntPtr engineApiPtr = EngineInterop.ReloadEngineCode(
                engineCode.DllPath, engineCode.DllShadowCopyPath);
            EngineApi engineApi = Marshal.PtrToStructure<EngineApi>(engineApiPtr);
            Engine = new Engine(engineApi, engineMemoryPtr);
            engineCode.DllLastWriteTimeUtc = File.GetLastWriteTimeUtc(engineCode.DllPath);

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

                case Win32.WindowMessage.MouseLeftButtonDown:
                    isMouseLeftDown = true;
                    return resultHandled;
                case Win32.WindowMessage.MouseLeftButtonUp:
                    isMouseLeftDown = false;
                    return resultHandled;
                case Win32.WindowMessage.MouseMiddleButtonDown:
                    isMouseMiddleDown = true;
                    return resultHandled;
                case Win32.WindowMessage.MouseMiddleButtonUp:
                    isMouseMiddleDown = false;
                    return resultHandled;
                case Win32.WindowMessage.MouseRightButtonDown:
                    isMouseRightDown = true;
                    return resultHandled;
                case Win32.WindowMessage.MouseRightButtonUp:
                    isMouseRightDown = false;
                    return resultHandled;
            }
            return Win32.DefWindowProc(hwnd, message, wParam, lParam);
        }

        private static void CaptureMouse()
        {
            shouldCaptureMouse = true;
        }

        private static void LogMessage(string message)
        {
            Debug.WriteLine(message);
        }

        internal static bool QueueAssetLoad(uint assetId, string absolutePath)
        {
            assetLoadRequests.Add(new AssetLoadRequest
            {
                AssetId = assetId,
                Path = absolutePath
            });

            return true;
        }

        private static bool QueueAssetLoadRelative(uint assetId, string relativePath)
        {
            return QueueAssetLoad(assetId, Path.Combine(assetsDirectoryPath, relativePath));
        }

        private static void WatchAssetFile(uint assetId, string relativePath)
        {
            var asset = new WatchedAsset
            {
                AssetId = assetId,
                Path = Path.Combine(assetsDirectoryPath, relativePath)
            };
            asset.LastUpdatedTimeUtc = File.GetLastWriteTimeUtc(asset.Path);
            watchedAssets.Add(asset);
        }

        private static EditorInputProxy GetInputState()
        {
            EditorInputProxy result = new EditorInputProxy();

            // query button state
            EditorInputButtons pressedButtons = 0;
            pressedButtons |= isMouseLeftDown ? EditorInputButtons.MouseLeft : 0;
            pressedButtons |= isMouseMiddleDown ? EditorInputButtons.MouseMiddle : 0;
            pressedButtons |= isMouseRightDown ? EditorInputButtons.MouseRight : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.Space) ? EditorInputButtons.KeySpace : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.D0) ? EditorInputButtons.Key0 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.D1) ? EditorInputButtons.Key1 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.D2) ? EditorInputButtons.Key2 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.D3) ? EditorInputButtons.Key3 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.D4) ? EditorInputButtons.Key4 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.D5) ? EditorInputButtons.Key5 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.D6) ? EditorInputButtons.Key6 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.D7) ? EditorInputButtons.Key7 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.D8) ? EditorInputButtons.Key8 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.D9) ? EditorInputButtons.Key9 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.A) ? EditorInputButtons.KeyA : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.B) ? EditorInputButtons.KeyB : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.C) ? EditorInputButtons.KeyC : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.D) ? EditorInputButtons.KeyD : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.E) ? EditorInputButtons.KeyE : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F) ? EditorInputButtons.KeyF : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.G) ? EditorInputButtons.KeyG : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.H) ? EditorInputButtons.KeyH : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.I) ? EditorInputButtons.KeyI : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.J) ? EditorInputButtons.KeyJ : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.K) ? EditorInputButtons.KeyK : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.L) ? EditorInputButtons.KeyL : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.M) ? EditorInputButtons.KeyM : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.N) ? EditorInputButtons.KeyN : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.O) ? EditorInputButtons.KeyO : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.P) ? EditorInputButtons.KeyP : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.Q) ? EditorInputButtons.KeyQ : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.R) ? EditorInputButtons.KeyR : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.S) ? EditorInputButtons.KeyS : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.T) ? EditorInputButtons.KeyT : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.U) ? EditorInputButtons.KeyU : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.V) ? EditorInputButtons.KeyV : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.W) ? EditorInputButtons.KeyW : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.X) ? EditorInputButtons.KeyX : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.Y) ? EditorInputButtons.KeyY : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.Z) ? EditorInputButtons.KeyZ : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.Escape) ? EditorInputButtons.KeyEscape : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.Enter) ? EditorInputButtons.KeyEnter : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.Right) ? EditorInputButtons.KeyRight : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.Left) ? EditorInputButtons.KeyLeft : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.Down) ? EditorInputButtons.KeyDown : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.Up) ? EditorInputButtons.KeyUp : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F1) ? EditorInputButtons.KeyF1 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F2) ? EditorInputButtons.KeyF2 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F3) ? EditorInputButtons.KeyF3 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F4) ? EditorInputButtons.KeyF4 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F5) ? EditorInputButtons.KeyF5 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F6) ? EditorInputButtons.KeyF6 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F7) ? EditorInputButtons.KeyF7 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F8) ? EditorInputButtons.KeyF8 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F9) ? EditorInputButtons.KeyF9 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F10) ? EditorInputButtons.KeyF10 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F11) ? EditorInputButtons.KeyF11 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.F12) ? EditorInputButtons.KeyF12 : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.LeftShift) ? EditorInputButtons.KeyLeftShift : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.LeftCtrl) ? EditorInputButtons.KeyLeftControl : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.RightShift) ? EditorInputButtons.KeyRightShift : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.RightCtrl) ? EditorInputButtons.KeyRightControl : 0;
            pressedButtons |= Keyboard.IsKeyDown(Key.LeftAlt) ? EditorInputButtons.KeyAlt : 0;

            // get mouse cursor position
            Win32.GetCursorPos(out var mousePosScreenSpaceWin32Point);
            Point mousePosScreenSpaceWpfPoint = new Point(
                mousePosScreenSpaceWin32Point.X, mousePosScreenSpaceWin32Point.Y);
            Point mousePosWindowSpaceWpfPoint =
                App.Current.MainWindow.PointFromScreen(mousePosScreenSpaceWpfPoint);

            var appWindow = App.Current.MainWindow;
            Point actualMousePosWindowSpace = mousePosWindowSpaceWpfPoint;
            Point virtualMousePosWindowSpace = actualMousePosWindowSpace;

            if (Win32.GetForegroundWindow() == mainWindowHwnd)
            {
                if (wasMouseCaptured)
                {
                    /*
                     * If we are capturing the mouse, we need to keep both the actual mouse position
                     * and a simulated 'virtual' mouse position. The actual mouse position is used for
                     * cursor offset calculations and the virtual mouse position is used when the
                     * cursor position is queried.
                     */
                    virtualMousePosWindowSpace = capturedMousePosWindowSpace;

                    if (!shouldCaptureMouse)
                    {
                        // move the cursor back to its original position when the mouse is released
                        Point capturedMousePosScreenSpace =
                            appWindow.PointToScreen(capturedMousePosWindowSpace);
                        Win32.SetCursorPos(
                            (int)capturedMousePosScreenSpace.X,
                            (int)capturedMousePosScreenSpace.Y);

                        actualMousePosWindowSpace = capturedMousePosWindowSpace;
                    }
                }

                for (int i = 0; i < viewportWindows.Count; i++)
                {
                    var viewportWindow = viewportWindows[i];
                    var vctx = viewportWindow.ViewContext;

                    if (actualMousePosWindowSpace.X < vctx.x
                        || actualMousePosWindowSpace.X >= vctx.x + vctx.width
                        || actualMousePosWindowSpace.Y < vctx.y
                        || actualMousePosWindowSpace.Y >= vctx.y + vctx.height)
                    {
                        continue;
                    }

                    result.activeViewState = vctx.viewState;
                    result.prevPressedButtons = (ulong)prevPressedButtons;
                    result.pressedButtons = (ulong)pressedButtons;
                    result.normalizedCursorPos.X =
                        (float)(virtualMousePosWindowSpace.X - vctx.x) / (float)vctx.width;
                    result.normalizedCursorPos.Y =
                        (float)(virtualMousePosWindowSpace.Y - vctx.y) / (float)vctx.height;
                    result.scrollOffset = nextMouseScrollOffsetY;

                    if (shouldCaptureMouse)
                    {
                        if (!wasMouseCaptured)
                        {
                            // store the cursor position so we can move the cursor back to it when
                            // the mouse is released
                            capturedMousePosWindowSpace = actualMousePosWindowSpace;
                        }

                        // calculate the center of the hovered viewport relative to the window
                        Point viewportCenterWindowSpace = new Point(
                            Math.Ceiling(vctx.x + (vctx.width * 0.5)),
                            Math.Ceiling(vctx.y + (vctx.height * 0.5)));

                        // convert the viewport center to screen space and move the cursor to it
                        Point viewportCenterScreenSpace =
                            appWindow.PointToScreen(viewportCenterWindowSpace);
                        Win32.SetCursorPos(
                            (int)viewportCenterScreenSpace.X, (int)viewportCenterScreenSpace.Y);

                        if (wasMouseCaptured)
                        {
                            result.cursorOffset = actualMousePosWindowSpace - viewportCenterWindowSpace;
                        }
                        else
                        {
                            // don't set the mouse offset on the first frame after we capture the mouse
                            // or there will be a big jump from the initial cursor position to the
                            // center of the viewport
                            result.cursorOffset = new Vector(0, 0);
                            Win32.SetCursor(IntPtr.Zero);
                        }
                    }
                    else
                    {
                        result.cursorOffset = actualMousePosWindowSpace - prevMousePosWindowSpace;
                    }
                    break;
                }
            }

            prevMousePosWindowSpace = actualMousePosWindowSpace;
            wasMouseCaptured = shouldCaptureMouse;
            shouldCaptureMouse = false;
            nextMouseScrollOffsetY = 0;
            prevPressedButtons = pressedButtons;

            return result;
        }

        private static void OnTick(object sender, EventArgs e)
        {
            DateTime now = DateTime.UtcNow;
            float deltaTime = (float)((now - lastTickTime).TotalSeconds);
            lastTickTime = now;

            EditorInputProxy input = GetInputState();

            if (!File.Exists(buildLockFilePath))
            {
                DateTime engineCodeDllLastWriteTime = File.GetLastWriteTimeUtc(engineCode.DllPath);
                if (engineCodeDllLastWriteTime > engineCode.DllLastWriteTimeUtc)
                {
                    IntPtr engineApiPtr = EngineInterop.ReloadEngineCode(engineCode.DllPath, engineCode.DllShadowCopyPath);
                    EngineApi engineApi = Marshal.PtrToStructure<EngineApi>(engineApiPtr);
                    Engine = new Engine(engineApi, engineMemoryPtr);
                    engineCode.DllLastWriteTimeUtc = engineCodeDllLastWriteTime;
                }

                DateTime editorCodeDllLastWriteTime = File.GetLastWriteTimeUtc(editorCode.DllPath);
                if (editorCodeDllLastWriteTime > editorCode.DllLastWriteTimeUtc)
                {
                    EngineInterop.ReloadEditorCode(editorCode.DllPath, editorCode.DllShadowCopyPath);
                    editorCode.DllLastWriteTimeUtc = editorCodeDllLastWriteTime;
                }
            }

            // invalidate watched assets that have changed
            foreach (var asset in watchedAssets)
            {
                DateTime lastWriteTimeUtc = File.GetLastWriteTimeUtc(asset.Path);
                if (lastWriteTimeUtc > asset.LastUpdatedTimeUtc)
                {
                    asset.LastUpdatedTimeUtc = lastWriteTimeUtc;
                    Engine.InvalidateAsset(asset.AssetId);
                }
            }

            // action any asset load requests
            for (int i = 0; i < assetLoadRequests.Count; i++)
            {
                var request = assetLoadRequests[i];
                try
                {
                    var data = File.ReadAllBytes(request.Path);
                    Engine.SetAssetData(request.AssetId, data);

                    assetLoadRequests.RemoveAt(i);
                    i--;
                }
                catch (IOException)
                {
                    // ignore and try again next tick
                }
            }

            EditorCore.Update(deltaTime, input);

            for (int i = 0; i < viewportWindows.Count; i++)
            {
                var viewportWindow = viewportWindows[i];
                if (viewportWindow.ViewContext.width == 0 || viewportWindow.ViewContext.height == 0)
                    continue;

                Win32.MakeGLContextCurrent(viewportWindow.DeviceContext, glRenderingContext);
                switch (viewportWindow.View)
                {
                    case EditorView.Scene:
                        EditorCore.RenderSceneView(ref viewportWindow.ViewContext);
                        break;
                    case EditorView.HeightmapPreview:
                        EditorCore.RenderHeightmapPreview(ref viewportWindow.ViewContext);
                        break;
                }
                Win32.SwapBuffers(viewportWindow.DeviceContext);
            }
        }

        internal static void Shutdown()
        {
            renderTimer.Stop();
            EditorCore.Shutdown();

            Win32.DestroyGLContext(glRenderingContext);
            Win32.DestroyWindow(dummyWindowHwnd);
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

            var viewportWindow = new EditorViewportWindow
            {
                Hwnd = hwnd,
                DeviceContext = deviceContext,
                View = view,
                ViewContext = new EditorViewContextProxy
                {
                    viewState = IntPtr.Zero,
                    x = x,
                    y = y,
                    width = width,
                    height = height
                }
            };
            viewportWindows.Add(viewportWindow);

            return viewportWindow;
        }

        internal static void DestroyViewportWindow(IntPtr hwnd)
        {
            Win32.DestroyWindow(hwnd);
        }
    }
}
