using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
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

        public void Reset()
        {
            FrameTime = TimeSpan.Zero;
            CoreUpdate = TimeSpan.Zero;
            RenderViewports = TimeSpan.Zero;
            RenderSceneView = TimeSpan.Zero;
        }

        public TimedBlock Measure_CoreUpdate() => new TimedBlock(elapsed => CoreUpdate += elapsed);
        public TimedBlock Measure_RenderViewports() => new TimedBlock(elapsed => RenderViewports += elapsed);
        public TimedBlock Measure_RenderSceneView() => new TimedBlock(elapsed => RenderSceneView += elapsed);
    }

    internal static class EditorPlatform
    {
        private static readonly string ViewportWindowClassName = "SierraViewportWindowClass";

        private static IntPtr appInstance = Win32.GetModuleHandle(null);
        private static Win32.WndProc viewportWndProc = new Win32.WndProc(ViewportWindowProc);

        private static IntPtr mainWindowHwnd;
        private static bool isRunning;

        const int PerfCounterHistoryFrameCount = 30;
        public static EditorPerformanceCounters[] PerfCountersByFrame;

        private static List<EditorViewportWindow> viewportWindows = new List<EditorViewportWindow>();

        private static bool isViewportHovered;
        private static bool wasMouseCaptured;
        private static long nextMouseScrollOffsetY;
        private static EditorInputButtons nextPressedButtons;

        private static string assetsDirectoryPath;

        private static PlatformLogMessage editorPlatformLogMessage = LogMessage;
        private static PlatformGetFileLastWriteTime editorPlatformGetFileLastWriteTime = GetFileLastWriteTime;
        private static PlatformGetFileSize editorPlatformGetFileSize = GetFileSize;
        private static PlatformReadEntireFile editorPlatformReadEntireFile = ReadEntireFile;

        internal static void Tick()
        {
            assetsDirectoryPath = Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory, "../../../data");
            string buildLockFilePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "build.lock");
            string coreDllPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "sierra_core.dll");
            string coreDllShadowCopyPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "sierra_core.shadow.dll");
            DateTime coreDllLastWriteTime = DateTime.MinValue;

            int appMemorySizeInBytes = 500 * 1024 * 1024;
            IntPtr appMemoryPtr = Win32.VirtualAlloc(IntPtr.Zero, (uint)appMemorySizeInBytes,
                Win32.AllocationType.Reserve | Win32.AllocationType.Commit,
                Win32.MemoryProtection.ReadWrite);
            EditorCore.Initialize(appMemoryPtr, appMemorySizeInBytes,
                editorPlatformLogMessage, editorPlatformGetFileLastWriteTime, editorPlatformGetFileSize,
                editorPlatformReadEntireFile, Win32.LoadLibrary, Win32.GetProcAddress, Win32.FreeLibrary);

            OpenGL.Initialize();

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

            PerfCountersByFrame = new EditorPerformanceCounters[PerfCounterHistoryFrameCount];
            for (int i = 0; i < PerfCounterHistoryFrameCount; i++)
            {
                PerfCountersByFrame[i] = new EditorPerformanceCounters();
            }
            int currentPerfCounterIndex = 0;
            DateTime lastTickTime = DateTime.UtcNow;

            Win32.Point capturedCursorPosScreenSpace = new Win32.Point();
            EditorInputButtons prevPressedButtons = 0;

            isRunning = true;
            while (isRunning)
            {
                EditorPerformanceCounters perfCounters = PerfCountersByFrame[currentPerfCounterIndex];
                currentPerfCounterIndex++;
                if (currentPerfCounterIndex == PerfCounterHistoryFrameCount)
                {
                    currentPerfCounterIndex = 0;
                }
                perfCounters.Reset();

                DateTime now = DateTime.UtcNow;
                perfCounters.FrameTime = now - lastTickTime;
                float deltaTime = (float)(perfCounters.FrameTime.TotalSeconds);
                lastTickTime = now;

                if (!File.Exists(buildLockFilePath))
                {
                    DateTime dllFileLastWriteTime = File.GetLastWriteTimeUtc(coreDllPath);
                    if (dllFileLastWriteTime > coreDllLastWriteTime)
                    {
                        if (EditorCore.ReloadCode(coreDllPath, coreDllShadowCopyPath))
                        {
                            coreDllLastWriteTime = dllFileLastWriteTime;
                        }
                    }
                }

                using (perfCounters.Measure_CoreUpdate())
                {
                    EditorCore.Update(deltaTime);
                }

                using (perfCounters.Measure_RenderViewports())
                {
                    bool isWindowActive = false;
                    Win32.Point cursorPosScreenSpace = new Win32.Point();
                    EditorInputButtons pressedButtons = 0;
                    if (mainWindowHwnd != IntPtr.Zero
                        && Win32.GetForegroundWindow() == mainWindowHwnd
                        && Win32.GetCursorPos(out cursorPosScreenSpace))
                    {
                        isWindowActive = true;
                        pressedButtons = nextPressedButtons;
                    }

                    bool isMouseCaptured = false;
                    bool isAnyViewportHovered = false;
                    long scrollOffsetY = nextMouseScrollOffsetY;
                    for (int i = 0; i < viewportWindows.Count; i++)
                    {
                        var viewportWindow = viewportWindows[i];
                        if (viewportWindow.ViewContext.Width == 0 || viewportWindow.ViewContext.Height == 0)
                            continue;

                        OpenGL.MakeDeviceCurrent(viewportWindow.DeviceContext);

                        var input = EditorInput.Disabled;
                        Win32.Rect viewportRect = new Win32.Rect();
                        if (isWindowActive && Win32.GetWindowRect(viewportWindow.Hwnd, out viewportRect)
                            && cursorPosScreenSpace.X >= viewportRect.Left
                            && cursorPosScreenSpace.X < viewportRect.Right
                            && cursorPosScreenSpace.Y >= viewportRect.Top
                            && cursorPosScreenSpace.Y < viewportRect.Bottom)
                        {
                            input.IsActive = true;
                            input.ScrollOffset = (float)scrollOffsetY;
                            input.PressedButtons = (ulong)pressedButtons;
                            input.PreviousPressedButtons = (ulong)prevPressedButtons;

                            Win32.Point virtualCursorPosScreenSpace = cursorPosScreenSpace;
                            if (wasMouseCaptured)
                            {
                                virtualCursorPosScreenSpace = capturedCursorPosScreenSpace;
                                input.CapturedCursorDelta.X
                                    = cursorPosScreenSpace.X - ((viewportRect.Left + viewportRect.Right) / 2);
                                input.CapturedCursorDelta.Y
                                    = cursorPosScreenSpace.Y - ((viewportRect.Top + viewportRect.Bottom) / 2);
                            }
                            input.CursorPos.X = virtualCursorPosScreenSpace.X - viewportRect.Left;
                            input.CursorPos.Y = viewportRect.Bottom - virtualCursorPosScreenSpace.Y;

                            isAnyViewportHovered = true;
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

                    isViewportHovered = isAnyViewportHovered;
                    Interlocked.Add(ref nextMouseScrollOffsetY, -scrollOffsetY);
                    prevPressedButtons = pressedButtons;

                    if (!isMouseCaptured && wasMouseCaptured)
                    {
                        Win32.SetCursorPos(capturedCursorPosScreenSpace.X, capturedCursorPosScreenSpace.Y);
                        wasMouseCaptured = false;
                    }
                }
            }

            OpenGL.Shutdown();
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
                    byte[] bytes = BitConverter.GetBytes(wParam.ToInt64());
                    short delta = BitConverter.ToInt16(bytes, 2);
                    Interlocked.Add(ref nextMouseScrollOffsetY, delta);
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

        internal static void Shutdown()
        {
            isRunning = false;
        }

        internal static void AttachToWindow(Window window)
        {
            var interopHelper = new WindowInteropHelper(window);
            mainWindowHwnd = interopHelper.EnsureHandle();

            window.PreviewKeyDown += HandleWindowKeyDown;
            window.PreviewKeyUp += HandleWindowKeyUp;
        }

        internal static EditorViewportWindow CreateViewportWindow(
            IntPtr parentHwnd, uint width, uint height, EditorView view)
        {
            IntPtr hwnd = Win32.CreateWindowEx(0, ViewportWindowClassName,
                "SierraViewportWindow", Win32.WindowStyles.Child | Win32.WindowStyles.Visible,
                0, 0, 1, 1, parentHwnd, IntPtr.Zero, appInstance, IntPtr.Zero);
            IntPtr deviceContext = Win32.GetDC(hwnd);
            OpenGL.ConfigureDevice(deviceContext);

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

        private static void HandleWindowKeyDown(object sender, KeyEventArgs e)
        {
            if (isViewportHovered)
            {
                if (!e.IsRepeat)
                {
                    nextPressedButtons |= GetInputButtonFromKey(e.Key);
                }
                e.Handled = true;
            }
        }

        private static void HandleWindowKeyUp(object sender, KeyEventArgs e)
        {
            if (isViewportHovered)
            {
                if (!e.IsRepeat)
                {
                    nextPressedButtons &= ~GetInputButtonFromKey(e.Key);
                }
                e.Handled = true;
            }
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
    }
}