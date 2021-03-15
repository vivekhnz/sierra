#include "Viewport.h"
#include "win32_editor_platform.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Windows;
using namespace System::Windows::Controls;
using namespace System::Windows::Input;
using namespace System::Windows::Media;
using namespace System::Windows::Media::Imaging;

ref class ViewportHwndHost : System::Windows::Interop::HwndHost
{
private:
    uint32 x;
    uint32 y;
    uint32 width;
    uint32 height;
    Terrain::Engine::Interop::EditorView view;

public:
    Win32ViewportWindow *window;

    ViewportHwndHost(uint32 x,
        uint32 y,
        uint32 width,
        uint32 height,
        Terrain::Engine::Interop::EditorView view)
    {
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
        this->view = view;
    }

protected:
    HandleRef BuildWindowCore(HandleRef hwndParent) override
    {
        HWND parentHwnd = (HWND)hwndParent.Handle.ToPointer();
        window = win32CreateViewportWindow(parentHwnd, x, y, width, height, view);
        return HandleRef(this, IntPtr(window->hwnd));
    }

    void DestroyWindowCore(HandleRef hwnd) override
    {
        // todo: release our Win32ViewportWindow so it can be reused by other viewports
        DestroyWindow((HWND)hwnd.Handle.ToPointer());
    }
};

namespace Terrain { namespace Engine { namespace Interop {
    Viewport::Viewport() : isInitialized(false)
    {
        if (System::ComponentModel::DesignerProperties::GetIsInDesignMode(this))
        {
            this->Background = gcnew SolidColorBrush(Colors::CornflowerBlue);
            return;
        }

        layoutRoot = gcnew Grid();
        AddChild(layoutRoot);

        this->Loaded += gcnew RoutedEventHandler(this, &Viewport::OnLoaded);
    }

    void Viewport::OnLoaded(Object ^ sender, RoutedEventArgs ^ args)
    {
        uint32 width = (uint32)Math::Max(layoutRoot->ActualWidth, 128.0);
        uint32 height = (uint32)Math::Max(layoutRoot->ActualHeight, 128.0);
        Point location = this->TranslatePoint(Point(0, 0), Application::Current->MainWindow);

        hwndHost = gcnew ViewportHwndHost(
            (uint32)location.X, (uint32)location.Y, width, height, View);
        layoutRoot->Children->Add(hwndHost);

        isInitialized = true;

        visualParent = (FrameworkElement ^) VisualTreeHelper::GetParent(this);
        if (visualParent != nullptr)
        {
            parentSizeChangedEventHandler = gcnew System::Windows::SizeChangedEventHandler(
                this, &Terrain::Engine::Interop::Viewport::OnParentSizeChanged);
            visualParent->SizeChanged += parentSizeChangedEventHandler;
        }
    }

    void Viewport::OnParentSizeChanged(Object ^ sender, SizeChangedEventArgs ^ args)
    {
        hwndHost->Width = args->NewSize.Width;
        hwndHost->Height = args->NewSize.Height;
    }

    void Viewport::OnRenderSizeChanged(SizeChangedInfo ^ info)
    {
        if (!isInitialized)
            return;

        uint32 width = (uint32)Math::Max(info->NewSize.Width, 128.0);
        uint32 height = (uint32)Math::Max(info->NewSize.Height, 128.0);
        Point location = this->TranslatePoint(Point(0, 0), Application::Current->MainWindow);

        hwndHost->Width = width;
        hwndHost->Height = height;
        hwndHost->window->vctx.x = location.X;
        hwndHost->window->vctx.y = location.Y;
        hwndHost->window->vctx.width = width;
        hwndHost->window->vctx.height = height;
    }

    Viewport::~Viewport()
    {
        if (!isInitialized)
            return;

        isInitialized = false;
        if (visualParent != nullptr)
        {
            visualParent->SizeChanged -= parentSizeChangedEventHandler;
        }
    }
}}}