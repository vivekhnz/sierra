using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;
using Sierra.Core;

namespace Sierra.Platform
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

    internal class TimedBlock : IDisposable
    {
        Stopwatch stopwatch;
        Action<TimeSpan> updateCounter;

        public TimedBlock(Action<TimeSpan> updateCounter)
        {
            stopwatch = Stopwatch.StartNew();
            this.updateCounter = updateCounter;
        }

        public void Dispose()
        {
            stopwatch.Stop();
            updateCounter(stopwatch.Elapsed);
        }
    }
    internal class EditorPerformanceCounters
    {
        public TimeSpan FrameTime;

        public TimeSpan CoreUpdate;
        public TimeSpan RenderViewports;
        public TimeSpan RenderSceneView;
        public TimeSpan UpdateBindings;

        public void Reset()
        {
            FrameTime = TimeSpan.Zero;
            CoreUpdate = TimeSpan.Zero;
            RenderViewports = TimeSpan.Zero;
            RenderSceneView = TimeSpan.Zero;
            UpdateBindings = TimeSpan.Zero;
        }

        public TimedBlock Measure_CoreUpdate() => new TimedBlock(elapsed => CoreUpdate += elapsed);
        public TimedBlock Measure_RenderViewports() => new TimedBlock(elapsed => RenderViewports += elapsed);
        public TimedBlock Measure_RenderSceneView() => new TimedBlock(elapsed => RenderSceneView += elapsed);
        public TimedBlock Measure_UpdateBindings() => new TimedBlock(elapsed => UpdateBindings += elapsed);
    }

    internal static class EditorPlatform
    {
        private class ReloadableCode
        {
            public string DllPath;
            public string DllShadowCopyPath;
            public DateTime DllLastWriteTimeUtc;
        }

        private static readonly string ViewportWindowClassName = "SierraOpenGLViewportWindowClass";

        private static IntPtr appInstance = Win32.GetModuleHandle(null);
        private static Win32.WndProc defWndProc = new Win32.WndProc(Win32.DefWindowProc);
        private static Win32.WndProc viewportWndProc = new Win32.WndProc(ViewportWindowProc);

        private static string buildLockFilePath;
        private static ReloadableCode editorCode;

        private static IntPtr mainWindowHwnd;
        private static IntPtr dummyWindowHwnd;
        private static IntPtr glRenderingContext;

        private static DateTime lastTickTime;

        private static bool wasMouseCaptured;
        private static Win32.Point capturedCursorPosScreenSpace;

        private static bool isViewportHovered;
        private static float nextMouseScrollOffsetY;
        private static EditorInputButtons nextPressedButtons;
        private static EditorInputButtons prevPressedButtons;

        private static List<EditorViewportWindow> viewportWindows = new List<EditorViewportWindow>();

        private static string assetsDirectoryPath;

        private static PlatformLogMessage editorPlatformLogMessage = LogMessage;
        private static PlatformGetFileLastWriteTime editorPlatformGetFileLastWriteTime = GetFileLastWriteTime;
        private static PlatformGetFileSize editorPlatformGetFileSize = GetFileSize;
        private static PlatformReadEntireFile editorPlatformReadEntireFile = ReadEntireFile;

        internal static void Initialize()
        {
            assetsDirectoryPath = Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory, "../../../data");
            buildLockFilePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "build.lock");
            editorCode = new ReloadableCode
            {
                DllPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "sierra_core.dll"),
                DllShadowCopyPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "sierra_core.shadow.dll")
            };

            int appMemorySizeInBytes = 500 * 1024 * 1024;
            IntPtr appMemoryPtr = Win32.VirtualAlloc(IntPtr.Zero, (uint)appMemorySizeInBytes,
                Win32.AllocationType.Reserve | Win32.AllocationType.Commit,
                Win32.MemoryProtection.ReadWrite);
            EditorCore.Initialize(appMemoryPtr, appMemorySizeInBytes,
                editorPlatformLogMessage, editorPlatformGetFileLastWriteTime, editorPlatformGetFileSize,
                editorPlatformReadEntireFile, Win32.LoadLibrary, Win32.GetProcAddress, Win32.FreeLibrary);

            var dummyWindowClass = new Win32.WindowClass
            {
                lpfnWndProc = Marshal.GetFunctionPointerForDelegate(defWndProc),
                hInstance = appInstance,
                lpszClassName = "SierraOpenGLDummyWindowClass"
            };
            Win32.RegisterClass(ref dummyWindowClass);
            dummyWindowHwnd = Win32.CreateWindowEx(0, dummyWindowClass.lpszClassName,
                "SierraOpenGLDummyWindow", 0, 0, 0, 100, 100, IntPtr.Zero, IntPtr.Zero, appInstance,
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
                    IntPtr cursor = IntPtr.Zero;
                    if (!wasMouseCaptured)
                    {
                        cursor = Win32.LoadCursor(IntPtr.Zero, Win32.Cursor.Arrow);
                    }
                    Win32.SetCursor(cursor);
                    return resultHandled;

                case Win32.WindowMessage.MouseLeftButtonDown:
                    nextPressedButtons |= EditorInputButtons.MouseLeft;
                    return resultHandled;
                case Win32.WindowMessage.MouseLeftButtonUp:
                    nextPressedButtons &= ~EditorInputButtons.MouseLeft;
                    return resultHandled;
                case Win32.WindowMessage.MouseMiddleButtonDown:
                    nextPressedButtons |= EditorInputButtons.MouseMiddle;
                    return resultHandled;
                case Win32.WindowMessage.MouseMiddleButtonUp:
                    nextPressedButtons &= ~EditorInputButtons.MouseMiddle;
                    return resultHandled;
                case Win32.WindowMessage.MouseRightButtonDown:
                    nextPressedButtons |= EditorInputButtons.MouseRight;
                    return resultHandled;
                case Win32.WindowMessage.MouseRightButtonUp:
                    nextPressedButtons &= ~EditorInputButtons.MouseRight;
                    return resultHandled;

                case Win32.WindowMessage.MouseWheel:
                    short delta = (short)(wParam.ToInt64() >> 16);
                    short direction = (short)(lParam.ToInt64() >> 16);
                    nextMouseScrollOffsetY += delta * direction;
                    return resultHandled;
            }
            return Win32.DefWindowProc(hwnd, message, wParam, lParam);
        }

        private static void LogMessage(string message)
        {
            Debug.WriteLine(message);
        }

        private static string GetAssetFilePath(string relativePath)
        {
            return Path.Combine(assetsDirectoryPath, relativePath);
        }
        private static long GetFileLastWriteTime(string relativePath)
        {
            string absolutePath = GetAssetFilePath(relativePath);
            DateTime lastWriteTimeUtc = File.GetLastWriteTimeUtc(absolutePath);
            return lastWriteTimeUtc.Ticks;
        }
        private static long GetFileSize(string relativePath)
        {
            string absolutePath = GetAssetFilePath(relativePath);
            return new FileInfo(absolutePath).Length;
        }
        private static void ReadEntireFile(string relativePath, ref byte bufferBaseAddress)
        {
            long fileSize = GetFileSize(relativePath);
            Debug.Assert(fileSize < int.MaxValue);

            var span = MemoryMarshal.CreateSpan(ref bufferBaseAddress, (int)fileSize);
            string absolutePath = GetAssetFilePath(relativePath);
            using (var stream = File.OpenRead(absolutePath))
            {
                int readBytes = stream.Read(span);
                Debug.Assert(readBytes == fileSize);
            }
        }

        internal static EditorInput GetInputStateForViewport(
            Win32.Rect viewportRect, Win32.Point cursorPosScreenSpace,
            float scrollOffset, EditorInputButtons pressedButtons, EditorInputButtons prevPressedButtons)
        {
            EditorInput result = EditorInput.Disabled;

            if (cursorPosScreenSpace.X >= viewportRect.Left
                && cursorPosScreenSpace.X < viewportRect.Right
                && cursorPosScreenSpace.Y >= viewportRect.Top
                && cursorPosScreenSpace.Y < viewportRect.Bottom)
            {
                result.IsActive = true;
                result.ScrollOffset = scrollOffset;
                result.PressedButtons = (ulong)pressedButtons;
                result.PreviousPressedButtons = (ulong)prevPressedButtons;

                Win32.Point virtualCursorPosScreenSpace = cursorPosScreenSpace;
                if (wasMouseCaptured)
                {
                    virtualCursorPosScreenSpace = capturedCursorPosScreenSpace;
                    result.CapturedCursorDelta.X
                        = cursorPosScreenSpace.X - ((viewportRect.Left + viewportRect.Right) / 2);
                    result.CapturedCursorDelta.Y
                        = cursorPosScreenSpace.Y - ((viewportRect.Top + viewportRect.Bottom) / 2);
                }
                result.CursorPos.X = virtualCursorPosScreenSpace.X - viewportRect.Left;
                result.CursorPos.Y = viewportRect.Bottom - virtualCursorPosScreenSpace.Y;
            }

            return result;
        }

        internal static void Tick(EditorPerformanceCounters perfCounters)
        {
            if (!File.Exists(buildLockFilePath))
            {
                DateTime editorCodeDllLastWriteTime = File.GetLastWriteTimeUtc(editorCode.DllPath);
                if (editorCodeDllLastWriteTime > editorCode.DllLastWriteTimeUtc)
                {
                    if (EditorCore.ReloadCode(editorCode.DllPath, editorCode.DllShadowCopyPath))
                    {
                        editorCode.DllLastWriteTimeUtc = editorCodeDllLastWriteTime;
                    }
                }
            }

            DateTime now = DateTime.UtcNow;
            float deltaTime = (float)((now - lastTickTime).TotalSeconds);
            lastTickTime = now;

            using (perfCounters.Measure_CoreUpdate())
            {
                EditorCore.Update(deltaTime);
            }

            using (perfCounters.Measure_RenderViewports())
            {
                var appWindow = App.Current.MainWindow;
                bool isWindowActive = false;
                Win32.Point cursorPosScreenSpace = new Win32.Point();
                EditorInputButtons pressedButtons = 0;
                if (appWindow != null)
                {
                    if (mainWindowHwnd == IntPtr.Zero)
                    {
                        var interopHelper = new WindowInteropHelper(appWindow);
                        mainWindowHwnd = interopHelper.EnsureHandle();
                    }
                    if (mainWindowHwnd != IntPtr.Zero
                        && Win32.GetForegroundWindow() == mainWindowHwnd
                        && Win32.GetCursorPos(out cursorPosScreenSpace))
                    {
                        isWindowActive = true;
                        pressedButtons = nextPressedButtons;
                    }
                }

                bool isMouseCaptured = false;
                isViewportHovered = false;
                for (int i = 0; i < viewportWindows.Count; i++)
                {
                    var viewportWindow = viewportWindows[i];
                    if (viewportWindow.ViewContext.Width == 0 || viewportWindow.ViewContext.Height == 0)
                        continue;

                    Win32.MakeGLContextCurrent(viewportWindow.DeviceContext, glRenderingContext);

                    var input = EditorInput.Disabled;
                    Win32.Rect viewportRect = new Win32.Rect();
                    if (isWindowActive && Win32.GetWindowRect(viewportWindow.Hwnd, out viewportRect))
                    {
                        input = GetInputStateForViewport(viewportRect, cursorPosScreenSpace,
                            nextMouseScrollOffsetY, pressedButtons, prevPressedButtons);
                        if (input.IsActive)
                        {
                            isViewportHovered = true;
                        }
                    }

                    switch (viewportWindow.View)
                    {
                        case EditorView.Scene:
                            using (perfCounters.Measure_RenderSceneView())
                            {
                                EditorCore.RenderSceneView(ref viewportWindow.ViewContext,
                                    deltaTime, ref input);
                            }
                            break;
                        case EditorView.HeightmapPreview:
                            EditorCore.RenderHeightmapPreview(ref viewportWindow.ViewContext,
                                deltaTime, ref input);
                            break;
                    }

                    if (input.IsMouseCaptured)
                    {
                        if (!wasMouseCaptured)
                        {
                            capturedCursorPosScreenSpace = cursorPosScreenSpace;
                            wasMouseCaptured = true;
                            Win32.SetCursor(IntPtr.Zero);
                        }

                        int dx = cursorPosScreenSpace.X - ((viewportRect.Left + viewportRect.Right) / 2);
                        int dy = cursorPosScreenSpace.Y - ((viewportRect.Top + viewportRect.Bottom) / 2);
                        Win32.GetCursorPos(out cursorPosScreenSpace);
                        Win32.SetCursorPos(cursorPosScreenSpace.X - dx, cursorPosScreenSpace.Y - dy);

                        isMouseCaptured = true;
                    }

                    Win32.SwapBuffers(viewportWindow.DeviceContext);
                }
                nextMouseScrollOffsetY = 0;
                prevPressedButtons = pressedButtons;

                if (!isMouseCaptured && wasMouseCaptured)
                {
                    Win32.SetCursorPos(capturedCursorPosScreenSpace.X, capturedCursorPosScreenSpace.Y);
                    wasMouseCaptured = false;
                }
            }
        }

        internal static void Shutdown()
        {
            Win32.DestroyGLContext(glRenderingContext);
            Win32.DestroyWindow(dummyWindowHwnd);
        }

        internal static EditorViewportWindow CreateViewportWindow(
            IntPtr parentHwnd, uint width, uint height, EditorView view)
        {
            IntPtr hwnd = Win32.CreateWindowEx(0, ViewportWindowClassName,
                "SierraOpenGLViewportWindow",
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

        private static EditorInputButtons GetInputButtonFromKey(Key key)
        {
            return (key) switch
            {
                Key.Space => EditorInputButtons.KeySpace,
                Key.D0 => EditorInputButtons.Key0,
                Key.D1 => EditorInputButtons.Key1,
                Key.D2 => EditorInputButtons.Key2,
                Key.D3 => EditorInputButtons.Key3,
                Key.D4 => EditorInputButtons.Key4,
                Key.D5 => EditorInputButtons.Key5,
                Key.D6 => EditorInputButtons.Key6,
                Key.D7 => EditorInputButtons.Key7,
                Key.D8 => EditorInputButtons.Key8,
                Key.D9 => EditorInputButtons.Key9,
                Key.A => EditorInputButtons.KeyA,
                Key.B => EditorInputButtons.KeyB,
                Key.C => EditorInputButtons.KeyC,
                Key.D => EditorInputButtons.KeyD,
                Key.E => EditorInputButtons.KeyE,
                Key.F => EditorInputButtons.KeyF,
                Key.G => EditorInputButtons.KeyG,
                Key.H => EditorInputButtons.KeyH,
                Key.I => EditorInputButtons.KeyI,
                Key.J => EditorInputButtons.KeyJ,
                Key.K => EditorInputButtons.KeyK,
                Key.L => EditorInputButtons.KeyL,
                Key.M => EditorInputButtons.KeyM,
                Key.N => EditorInputButtons.KeyN,
                Key.O => EditorInputButtons.KeyO,
                Key.P => EditorInputButtons.KeyP,
                Key.Q => EditorInputButtons.KeyQ,
                Key.R => EditorInputButtons.KeyR,
                Key.S => EditorInputButtons.KeyS,
                Key.T => EditorInputButtons.KeyT,
                Key.U => EditorInputButtons.KeyU,
                Key.V => EditorInputButtons.KeyV,
                Key.W => EditorInputButtons.KeyW,
                Key.X => EditorInputButtons.KeyX,
                Key.Y => EditorInputButtons.KeyY,
                Key.Z => EditorInputButtons.KeyZ,
                Key.Escape => EditorInputButtons.KeyEscape,
                Key.Enter => EditorInputButtons.KeyEnter,
                Key.Right => EditorInputButtons.KeyRight,
                Key.Left => EditorInputButtons.KeyLeft,
                Key.Down => EditorInputButtons.KeyDown,
                Key.Up => EditorInputButtons.KeyUp,
                Key.F1 => EditorInputButtons.KeyF1,
                Key.F2 => EditorInputButtons.KeyF2,
                Key.F3 => EditorInputButtons.KeyF3,
                Key.F4 => EditorInputButtons.KeyF4,
                Key.F5 => EditorInputButtons.KeyF5,
                Key.F6 => EditorInputButtons.KeyF6,
                Key.F7 => EditorInputButtons.KeyF7,
                Key.F8 => EditorInputButtons.KeyF8,
                Key.F9 => EditorInputButtons.KeyF9,
                Key.F10 => EditorInputButtons.KeyF10,
                Key.F11 => EditorInputButtons.KeyF11,
                Key.F12 => EditorInputButtons.KeyF12,
                Key.LeftShift => EditorInputButtons.KeyLeftShift,
                Key.LeftCtrl => EditorInputButtons.KeyLeftControl,
                Key.LeftAlt => EditorInputButtons.KeyAlt,
                Key.Delete => EditorInputButtons.KeyDelete,
                _ => 0
            };
        }

        internal static void HandleWindowKeyDown(object sender, KeyEventArgs e)
        {
            if (isViewportHovered)
            {
                nextPressedButtons |= GetInputButtonFromKey(e.Key);
                e.Handled = true;
            }
        }

        internal static void HandleWindowKeyUp(object sender, KeyEventArgs e)
        {
            if (isViewportHovered)
            {
                nextPressedButtons &= ~GetInputButtonFromKey(e.Key);
                e.Handled = true;
            }
        }
    }
}