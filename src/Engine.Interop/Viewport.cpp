#include "Viewport.h"

#include "EngineInterop.hpp"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Windows;
using namespace System::Windows::Controls;
using namespace System::Windows::Input;
using namespace System::Windows::Media;
using namespace System::Windows::Media::Imaging;

ref class ViewportHwndHost : System::Windows::Interop::HwndHost
{
public:
    HDC deviceContext = 0;

protected:
    HandleRef BuildWindowCore(HandleRef hwndParent) override
    {
        HWND parentHwnd = (HWND)hwndParent.Handle.ToPointer();
        Win32ViewportWindow window = win32CreateViewportWindow(parentHwnd);
        deviceContext = window.deviceContext;
        return HandleRef(this, IntPtr(window.hwnd));
    }

    void DestroyWindowCore(HandleRef hwnd) override
    {
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
        hwndHost = gcnew ViewportHwndHost();
        layoutRoot->Children->Add(hwndHost);

        vctx = EngineInterop::CreateView(hwndHost->deviceContext);

        uint32 width = (uint32)Math::Max(layoutRoot->ActualWidth, 128.0);
        uint32 height = (uint32)Math::Max(layoutRoot->ActualHeight, 128.0);
        Point location = this->TranslatePoint(Point(0, 0), Application::Current->MainWindow);
        vctx->resize(location.X, location.Y, width, height);

        vctx->setEditorView(View);
        EngineInterop::LinkViewportToEditorView(vctx, View);

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
        vctx->resize(location.X, location.Y, width, height);

        hwndHost->Width = width;
        hwndHost->Height = height;
    }

    Viewport::~Viewport()
    {
        if (!isInitialized)
            return;

        isInitialized = false;
        EngineInterop::DetachView(vctx);

        if (visualParent != nullptr)
        {
            visualParent->SizeChanged -= parentSizeChangedEventHandler;
        }
    }
}}}