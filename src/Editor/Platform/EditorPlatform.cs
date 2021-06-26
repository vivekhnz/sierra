using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;
using Terrain.Editor.Core;
using Terrain.Editor.Engine;

namespace Terrain.Editor.Platform
{
    public enum EditorView
    {
        None = 0,
        Scene = 1,
        HeightmapPreview = 2
    };

    internal class EditorViewportWindow
    {
        public IntPtr Hwnd;
        public IntPtr DeviceContext;
        public EditorView View;
        public EditorViewContext ViewContext;
    }

    internal static class EditorPlatform
    {
        private class AssetLoadRequest
        {
            public IntPtr AssetHandle;
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

        private static PlatformCaptureMouse editorPlatformCaptureMouse = CaptureMouse;
        private static PlatformLogMessage editorPlatformLogMessage = LogMessage;
        private static PlatformQueueAssetLoad editorPlatformQueueAssetLoad = QueueAssetLoadRelative;
        private static PlatformWatchAssetFile editorPlatformWatchAssetFile = WatchAssetFile;

        public static bool IsViewportHovered { get; private set; } = false;

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

            int editorMemorySizeInBytes = 32 * 1024 * 1024;
            int engineMemorySizeInBytes = 480 * 1024 * 1024;
            int appMemorySizeInBytes = editorMemorySizeInBytes + engineMemorySizeInBytes;
            IntPtr appMemoryPtr = Win32.VirtualAlloc(IntPtr.Zero, (uint)appMemorySizeInBytes,
                Win32.AllocationType.Reserve | Win32.AllocationType.Commit,
                Win32.MemoryProtection.ReadWrite);
            IntPtr editorMemoryDataPtr = appMemoryPtr;
            IntPtr engineMemoryDataPtr = editorMemoryDataPtr + editorMemorySizeInBytes;

            IntPtr engineMemoryPtr = TerrainEngine.Initialize(engineMemoryDataPtr, engineMemorySizeInBytes,
                editorPlatformLogMessage, editorPlatformQueueAssetLoad,
                editorPlatformWatchAssetFile, Win32.LoadLibrary, Win32.GetProcAddress,
                Win32.FreeLibrary);
            EditorCore.Initialize(editorMemoryDataPtr, editorMemorySizeInBytes,
                editorPlatformCaptureMouse, engineMemoryPtr, Win32.LoadLibrary,
                Win32.GetProcAddress, Win32.FreeLibrary);

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

            TerrainEngine.ReloadCode(engineCode.DllPath, engineCode.DllShadowCopyPath);
            EditorCore.UpdateEngineApi(TerrainEngine.EngineApiPtr);
            engineCode.DllLastWriteTimeUtc = File.GetLastWriteTimeUtc(engineCode.DllPath);

            lastTickTime = DateTime.UtcNow;
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

        internal static bool QueueAssetLoad(IntPtr assetHandle, string absolutePath)
        {
            assetLoadRequests.Add(new AssetLoadRequest
            {
                AssetHandle = assetHandle,
                Path = absolutePath
            });

            return true;
        }

        private static bool QueueAssetLoadRelative(IntPtr assetHandle, string relativePath)
        {
            return QueueAssetLoad(assetHandle, Path.Combine(assetsDirectoryPath, relativePath));
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

        private static EditorInput GetInputState()
        {
            EditorInput result = new EditorInput();

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
            Point actualMousePosWindowSpace = new Point();
            Point virtualMousePosWindowSpace = actualMousePosWindowSpace;

            var appWindow = App.Current.MainWindow;
            if (appWindow != null)
            {
                actualMousePosWindowSpace = appWindow.PointFromScreen(mousePosScreenSpaceWpfPoint);
                virtualMousePosWindowSpace = actualMousePosWindowSpace;

                if (mainWindowHwnd == IntPtr.Zero)
                {
                    var interopHelper = new WindowInteropHelper(appWindow);
                    mainWindowHwnd = interopHelper.EnsureHandle();
                }
                if (mainWindowHwnd != IntPtr.Zero && Win32.GetForegroundWindow() == mainWindowHwnd)
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

                        if (actualMousePosWindowSpace.X < vctx.X
                            || actualMousePosWindowSpace.X >= vctx.X + vctx.Width
                            || actualMousePosWindowSpace.Y < vctx.Y
                            || actualMousePosWindowSpace.Y >= vctx.Y + vctx.Height)
                        {
                            continue;
                        }

                        result.ActiveViewState = vctx.ViewState;
                        result.PreviousPressedButtons = (ulong)prevPressedButtons;
                        result.PressedButtons = (ulong)pressedButtons;
                        result.NormalizedCursorPos.X =
                            (float)(virtualMousePosWindowSpace.X - vctx.X) / (float)vctx.Width;
                        result.NormalizedCursorPos.Y =
                            (float)(virtualMousePosWindowSpace.Y - vctx.Y) / (float)vctx.Height;
                        result.ScrollOffset = nextMouseScrollOffsetY;

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
                                Math.Ceiling(vctx.X + (vctx.Width * 0.5)),
                                Math.Ceiling(vctx.Y + (vctx.Height * 0.5)));

                            // convert the viewport center to screen space and move the cursor to it
                            Point viewportCenterScreenSpace =
                                appWindow.PointToScreen(viewportCenterWindowSpace);
                            Win32.SetCursorPos(
                                (int)viewportCenterScreenSpace.X, (int)viewportCenterScreenSpace.Y);

                            if (wasMouseCaptured)
                            {
                                result.CursorOffset.X = (float)actualMousePosWindowSpace.X
                                    - (float)viewportCenterWindowSpace.X;
                                result.CursorOffset.Y = (float)actualMousePosWindowSpace.Y
                                    - (float)viewportCenterWindowSpace.Y;
                            }
                            else
                            {
                                // don't set the mouse offset on the first frame after we capture the mouse
                                // or there will be a big jump from the initial cursor position to the
                                // center of the viewport
                                result.CursorOffset = new Vector2(0, 0);
                                Win32.SetCursor(IntPtr.Zero);
                            }
                        }
                        else
                        {
                            result.CursorOffset.X = (float)actualMousePosWindowSpace.X
                                - (float)prevMousePosWindowSpace.X;
                            result.CursorOffset.Y = (float)actualMousePosWindowSpace.Y
                                - (float)prevMousePosWindowSpace.Y;
                        }
                        break;
                    }
                }
            }

            prevMousePosWindowSpace = actualMousePosWindowSpace;
            wasMouseCaptured = shouldCaptureMouse;
            shouldCaptureMouse = false;
            nextMouseScrollOffsetY = 0;
            prevPressedButtons = pressedButtons;

            return result;
        }

        internal static void Tick()
        {
            if (!File.Exists(buildLockFilePath))
            {
                DateTime engineCodeDllLastWriteTime = File.GetLastWriteTimeUtc(engineCode.DllPath);
                if (engineCodeDllLastWriteTime > engineCode.DllLastWriteTimeUtc)
                {
                    TerrainEngine.ReloadCode(engineCode.DllPath, engineCode.DllShadowCopyPath);
                    EditorCore.UpdateEngineApi(TerrainEngine.EngineApiPtr);
                    engineCode.DllLastWriteTimeUtc = engineCodeDllLastWriteTime;
                }

                DateTime editorCodeDllLastWriteTime = File.GetLastWriteTimeUtc(editorCode.DllPath);
                if (editorCodeDllLastWriteTime > editorCode.DllLastWriteTimeUtc)
                {
                    if (EditorCore.ReloadCode(editorCode.DllPath, editorCode.DllShadowCopyPath))
                    {
                        editorCode.DllLastWriteTimeUtc = editorCodeDllLastWriteTime;
                    }
                }
            }

            // invalidate watched assets that have changed
            foreach (var asset in watchedAssets)
            {
                DateTime lastWriteTimeUtc = File.GetLastWriteTimeUtc(asset.Path);
                if (lastWriteTimeUtc > asset.LastUpdatedTimeUtc)
                {
                    asset.LastUpdatedTimeUtc = lastWriteTimeUtc;
                    TerrainEngine.InvalidateAsset(asset.AssetId);
                }
            }

            // action any asset load requests
            for (int i = 0; i < assetLoadRequests.Count; i++)
            {
                var request = assetLoadRequests[i];
                try
                {
                    var data = File.ReadAllBytes(request.Path);
                    TerrainEngine.SetAssetData(request.AssetHandle, data);

                    assetLoadRequests.RemoveAt(i);
                    i--;
                }
                catch (IOException)
                {
                    // ignore and try again next tick
                }
            }

            DateTime now = DateTime.UtcNow;
            float deltaTime = (float)((now - lastTickTime).TotalSeconds);
            lastTickTime = now;
            EditorInput input = GetInputState();

            IsViewportHovered = input.ActiveViewState != IntPtr.Zero;
            EditorCore.Update(deltaTime, ref input);

            for (int i = 0; i < viewportWindows.Count; i++)
            {
                var viewportWindow = viewportWindows[i];
                if (viewportWindow.ViewContext.Width == 0 || viewportWindow.ViewContext.Height == 0)
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
            TerrainEngine.Shutdown();

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
                ViewContext = new EditorViewContext
                {
                    ViewState = IntPtr.Zero,
                    X = x,
                    Y = y,
                    Width = width,
                    Height = height
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
