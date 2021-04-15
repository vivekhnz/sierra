using PInvoke;
using System;
using Terrain.Engine.Interop;

namespace Terrain.Editor
{
    internal static class EditorPlatform
    {
        internal static void Initialize()
        {
            EngineInterop.InitializeEngine();
        }

        internal static void Shutdown()
        {
            EngineInterop.Shutdown();
        }

        internal static EditorPlatformViewportWindow CreateViewportWindow(IntPtr parentHwnd,
            uint x, uint y, uint width, uint height, EditorView view)
        {
            return EngineInterop.CreateViewportWindow(parentHwnd, x, y, width, height, view);
        }

        internal static void DestroyViewportWindow(IntPtr hwnd)
        {
            User32.DestroyWindow(hwnd);
        }

        internal static void ResizeViewportWindow(IntPtr windowPtr,
            uint x, uint y, uint width, uint height)
        {
            EngineInterop.ResizeViewportWindow(windowPtr, x, y, width, height);
        }
    }
}
