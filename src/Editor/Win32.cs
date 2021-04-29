using System;
using System.Runtime.InteropServices;

namespace Terrain.Editor
{
    internal static class Win32
    {
        // kernel32.dll

        [Flags]
        internal enum AllocationType : uint
        {
            Commit = 0x1000,
            Reserve = 0x2000,
            Decommit = 0x4000,
            Release = 0x8000,
            Reset = 0x80000,
            Physical = 0x400000,
            TopDown = 0x100000,
            WriteWatch = 0x200000,
            LargePages = 0x20000000
        }

        [Flags]
        internal enum MemoryProtection : uint
        {
            Execute = 0x10,
            ExecuteRead = 0x20,
            ExecuteReadWrite = 0x40,
            ExecuteWriteCopy = 0x80,
            NoAccess = 0x01,
            ReadOnly = 0x02,
            ReadWrite = 0x04,
            WriteCopy = 0x08,
            GuardModifierflag = 0x100,
            NoCacheModifierflag = 0x200,
            WriteCombineModifierflag = 0x400
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        internal static extern IntPtr VirtualAlloc(IntPtr lpAddress, uint dwSize,
            AllocationType flAllocationType, MemoryProtection flProtect);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        internal static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Ansi)]
        internal static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("kernel32.dll", SetLastError = true)]
        internal static extern bool FreeLibrary(IntPtr hModule);

        [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        internal static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        // user32.dll

        internal delegate IntPtr WndProc(
            IntPtr hwnd, WindowMessage message, IntPtr wParam, IntPtr lParam);

        [Flags]
        internal enum WindowClassStyles : uint
        {
            ByteAlignClient = 0x1000,
            ByteAlignWindow = 0x2000,
            ClassDC = 0x40,
            DoubleClicks = 0x8,
            DropShadow = 0x20000,
            GlobalClass = 0x4000,
            HorizontalRedraw = 0x2,
            NoClose = 0x200,
            OwnDC = 0x20,
            ParentDC = 0x80,
            SaveBits = 0x800,
            VerticalRedraw = 0x1
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct WindowClass
        {
            public WindowClassStyles style;
            public IntPtr lpfnWndProc;
            public int cbClsExtra;
            public int cbWndExtra;
            public IntPtr hInstance;
            public IntPtr hIcon;
            public IntPtr hCursor;
            public IntPtr hbrBackground;
            public string lpszMenuName;
            public string lpszClassName;
        }

        [Flags]
        internal enum WindowStyles : uint
        {
            Border = 0x800000,
            Caption = 0xc00000,
            Child = 0x40000000,
            ClipChildren = 0x2000000,
            ClipSiblings = 0x4000000,
            Disabled = 0x8000000,
            DialogFrame = 0x400000,
            Group = 0x20000,
            HorizontalScrollBar = 0x100000,
            Maximize = 0x1000000,
            MaximizeBox = 0x10000,
            Minimize = 0x20000000,
            MinimizeBox = 0x20000,
            Overlapped = 0x0,
            OverlappedWindow = Overlapped | Caption | SysMenu | SizeFrame | MinimizeBox | MaximizeBox,
            Popup = 0x80000000u,
            PopupWindow = Popup | Border | SysMenu,
            SizeFrame = 0x40000,
            SysMenu = 0x80000,
            TabStop = 0x10000,
            Visible = 0x10000000,
            VerticalScrollBar = 0x200000
        }

        internal enum WindowMessage : uint
        {
            SetCursor = 0x0020,
            MouseWheel = 0x020A,
            MouseLeftButtonDown = 0x0201,
            MouseLeftButtonUp = 0x0202,
            MouseRightButtonDown = 0x0204,
            MouseRightButtonUp = 0x0205,
            MouseMiddleButtonDown = 0x0207,
            MouseMiddleButtonUp = 0x0208
        }

        internal enum Cursor
        {
            Arrow = 32512,
            IBeam = 32513,
            Wait = 32514,
            Cross = 32515,
            UpArrow = 32516,
            Size = 32640,
            Icon = 32641,
            SizeNWSE = 32642,
            SizeNESW = 32643,
            SizeWE = 32644,
            SizeNS = 32645,
            SizeAll = 32646,
            No = 32648,
            Hand = 32649,
            AppStarting = 32650,
            Help = 32651
        }

        internal enum VirtualKey
        {
            MouseLeft = 0x01,
            MouseRight = 0x02,
            MouseMiddle = 0x04,
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct Point
        {
            public int X;
            public int Y;
        }

        [DllImport("user32.dll", SetLastError = true)]
        internal static extern IntPtr DefWindowProc(IntPtr hwnd, WindowMessage message,
            IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        internal static extern ushort RegisterClass(ref WindowClass lpWndClass);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        internal static extern IntPtr CreateWindowEx(ulong dwExStyle, string lpClassName,
           string lpWindowName, WindowStyles dwStyle, int x, int y, int nWidth, int nHeight,
           IntPtr hWndParent, IntPtr hMenu, IntPtr hInstance, IntPtr lpParam);

        [DllImport("user32.dll", SetLastError = true)]
        internal static extern IntPtr GetDC(IntPtr hWnd);

        [DllImport("user32.dll", SetLastError = true)]
        internal static extern bool DestroyWindow(IntPtr hWnd);

        [DllImport("user32.dll", SetLastError = true)]
        internal static extern IntPtr LoadCursor(IntPtr hInstance, Cursor lpCursorName);

        [DllImport("user32.dll", SetLastError = true)]
        internal static extern IntPtr SetCursor(IntPtr handle);

        [DllImport("user32.dll", SetLastError = true)]
        internal static extern IntPtr GetForegroundWindow();

        [DllImport("user32.dll", SetLastError = true)]
        internal static extern bool GetCursorPos(out Point lpPoint);

        [DllImport("user32.dll", SetLastError = true)]
        internal static extern bool SetCursorPos(int X, int Y);

        // gdi32.dll

        [Flags]
        internal enum PixelFormatDescriptorFlags : uint
        {
            DoubleBuffer = 0x00000001,
            Stereo = 0x00000002,
            DrawToWindow = 0x00000004,
            DrawToBitmap = 0x00000008,
            SupportGdi = 0x00000010,
            SupportOpenGL = 0x00000020,
            GenericFormat = 0x00000040,
            NeedPalette = 0x00000080,
            NeedSystemPalette = 0x00000100,
            SwapExchange = 0x00000200,
            SwapCopy = 0x00000400,
            SwapLayerBuffers = 0x00000800,
            GenericAccelerated = 0x00001000,
            SupportDirectDraw = 0x00002000,
            Direct3DAccelerated = 0x00004000,
            SupportComposition = 0x00008000
        }
        internal enum PixelFormatDescriptorPixelType
        {
            RGBA = 0,
            ColorIndex = 1
        }
        internal enum PixelFormatDescriptorLayerType
        {
            UnderlayPlane = -1,
            MainPlane = 0,
            OverlayPlane = 1
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct PixelFormatDescriptor
        {
            public ushort Size;
            public ushort Version;
            public PixelFormatDescriptorFlags Flags;
            public PixelFormatDescriptorPixelType PixelType;
            public byte ColorBits;
            public byte RedBits;
            public byte RedShift;
            public byte GreenBits;
            public byte GreenShift;
            public byte BlueBits;
            public byte BlueShift;
            public byte AlphaBits;
            public byte AlphaShift;
            public byte AccumBits;
            public byte AccumRedBits;
            public byte AccumGreenBits;
            public byte AccumBlueBits;
            public byte AccumAlphaBits;
            public byte DepthBits;
            public byte StencilBits;
            public byte AuxBuffers;
            public PixelFormatDescriptorLayerType LayerType;
            private byte Reserved;
            public uint LayerMask;
            public uint VisibleMask;
            public uint DamageMask;
        }

        [DllImport("gdi32.dll", SetLastError = true)]
        internal static extern int ChoosePixelFormat(IntPtr hDC, ref PixelFormatDescriptor ppfd);

        [DllImport("gdi32.dll", SetLastError = true)]
        internal static extern int SetPixelFormat(IntPtr hDC, int pixelFormat,
            ref PixelFormatDescriptor ppfd);

        [DllImport("gdi32.dll", SetLastError = true)]
        internal static extern int SwapBuffers(IntPtr hDC);

        // opengl32.dll

        [DllImport("opengl32.dll", EntryPoint = "wglCreateContext", SetLastError = true)]
        internal static extern IntPtr CreateGLContext(IntPtr hDC);

        [DllImport("opengl32.dll", EntryPoint = "wglMakeCurrent", SetLastError = true)]
        internal static extern bool MakeGLContextCurrent(IntPtr hDC, IntPtr renderingContext);

        [DllImport("opengl32.dll", EntryPoint = "wglDeleteContext", SetLastError = true)]
        internal static extern IntPtr DestroyGLContext(IntPtr renderingContext);
    }
}
