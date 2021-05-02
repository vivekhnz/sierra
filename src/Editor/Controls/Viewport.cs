using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Windows.Media;
using Terrain.Editor.Platform;

namespace Terrain.Editor.Controls
{
    public class Viewport : UserControl
    {
        private class ViewportHwndHost : HwndHost
        {
            uint x;
            uint y;
            uint width;
            uint height;
            EditorView view;

            private EditorViewportWindow window;

            internal ViewportHwndHost(uint x, uint y, uint width, uint height, EditorView view)
            {
                this.x = x;
                this.y = y;
                this.width = width;
                this.height = height;
                this.view = view;
            }

            protected override HandleRef BuildWindowCore(HandleRef hwndParent)
            {
                window = EditorPlatform.CreateViewportWindow(
                    hwndParent.Handle, x, y, width, height, view);
                return new HandleRef(this, window.Hwnd);
            }

            protected override void DestroyWindowCore(HandleRef hwnd)
            {
                // todo: remove our EditorViewportWindow
                EditorPlatform.DestroyViewportWindow(hwnd.Handle);
            }

            internal void ResizeWindow(Point location, uint width, uint height)
            {
                Width = width;
                Height = height;
                window.ViewContext.X = (uint)location.X;
                window.ViewContext.Y = (uint)location.Y;
                window.ViewContext.Width = width;
                window.ViewContext.Height = height;
            }
        }

        bool isInitialized;

        Grid layoutRoot;
        ViewportHwndHost hwndHost;

        FrameworkElement visualParent;

        public EditorView View { get; set; }

        public Viewport()
        {
            if (DesignerProperties.GetIsInDesignMode(this))
            {
                Background = new SolidColorBrush(Colors.CornflowerBlue);
                return;
            }

            layoutRoot = new Grid();
            AddChild(layoutRoot);

            Loaded += OnLoaded;
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            uint width = (uint)Math.Max(layoutRoot.ActualWidth, 128.0);
            uint height = (uint)Math.Max(layoutRoot.ActualHeight, 128.0);
            Point location = TranslatePoint(new Point(0, 0), Application.Current.MainWindow);

            hwndHost = new ViewportHwndHost(
                (uint)location.X, (uint)location.Y, width, height, View);
            layoutRoot.Children.Add(hwndHost);

            isInitialized = true;

            visualParent = (FrameworkElement)VisualTreeHelper.GetParent(this);
            if (visualParent != null)
            {
                visualParent.SizeChanged += OnParentSizeChanged;
            }
        }

        private void OnParentSizeChanged(object sender, SizeChangedEventArgs e)
        {
            hwndHost.Width = e.NewSize.Width;
            hwndHost.Height = e.NewSize.Height;
        }

        protected override void OnVisualParentChanged(DependencyObject oldParent)
        {
            if (visualParent != null)
            {
                visualParent.SizeChanged -= OnParentSizeChanged;
            }

            if (isInitialized)
            {
                visualParent = (FrameworkElement)VisualTreeHelper.GetParent(this);
                if (visualParent != null)
                {
                    visualParent.SizeChanged += OnParentSizeChanged;
                }
            }

            base.OnVisualParentChanged(oldParent);
        }

        protected override void OnRenderSizeChanged(SizeChangedInfo info)
        {
            if (!isInitialized)
                return;

            uint width = (uint)Math.Max(info.NewSize.Width, 128.0);
            uint height = (uint)Math.Max(info.NewSize.Height, 128.0);
            Point location = this.TranslatePoint(new Point(0, 0), Application.Current.MainWindow);

            hwndHost.ResizeWindow(location, width, height);

            base.OnRenderSizeChanged(info);
        }
    }
}
