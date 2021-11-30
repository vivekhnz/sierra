using System;
using System.Runtime.InteropServices;

namespace Sierra.Platform
{
    internal static class OpenGL
    {
        private static Win32.WndProc defWndProc = new Win32.WndProc(Win32.DefWindowProc);

        private static IntPtr dummyWindowHwnd;
        private static IntPtr glRenderingContext;
        private static IntPtr currentDeviceContext;

        internal static void Initialize()
        {
            IntPtr appInstance = Win32.GetModuleHandle(null);
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
            MakeDeviceCurrent(dummyDeviceContext);
        }

        internal static void ConfigureDevice(IntPtr deviceContext)
        {
            ConfigureDeviceContextForOpenGL(deviceContext);
        }

        internal static void MakeDeviceCurrent(IntPtr deviceContext)
        {
            if (currentDeviceContext == deviceContext) return;

            Win32.MakeGLContextCurrent(deviceContext, glRenderingContext);
            currentDeviceContext = deviceContext;
        }

        internal static void Shutdown()
        {
            Win32.DestroyGLContext(glRenderingContext);
            Win32.DestroyWindow(dummyWindowHwnd);
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
    }
}