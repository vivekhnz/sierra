#include "Viewport.h"

#include <iostream>
#include "EngineInterop.hpp"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Windows;
using namespace System::Windows::Controls;

namespace Terrain { namespace Engine { namespace Interop {
    Viewport::Viewport() : isInitialized(false)
    {
        isInDesignMode = System::ComponentModel::DesignerProperties::GetIsInDesignMode(this);
        if (isInDesignMode)
        {
            this->Background = gcnew SolidColorBrush(Colors::CornflowerBlue);
            return;
        }
        this->Focusable = true;

        unfocusedBrush = gcnew SolidColorBrush(Color::FromRgb(24, 24, 24));
        focusedBrush = gcnew SolidColorBrush(Color::FromRgb(0, 127, 212));

        layoutRoot = gcnew Grid();
        AddChild(layoutRoot);

        bitmap = gcnew WriteableBitmap(1280, 720, 96, 96, PixelFormats::Pbgra32, nullptr);
        image = gcnew Image();
        image->Source = bitmap;
        image->RenderTransformOrigin = Point(0.5, 0.5);
        image->RenderTransform = gcnew ScaleTransform(1.0, -1.0);
        image->Stretch = Stretch::UniformToFill;
        layoutRoot->Children->Add(image);

        focusBorder = gcnew Border();
        focusBorder->BorderBrush = unfocusedBrush;
        focusBorder->BorderThickness = Thickness(1);
        layoutRoot->Children->Add(focusBorder);

        hoverBorder = gcnew Border();
        hoverBorder->BorderBrush = gcnew SolidColorBrush(Color::FromArgb(16, 255, 255, 255));
        hoverBorder->BorderThickness = Thickness(1);
        hoverBorder->Visibility = Windows::Visibility::Hidden;
        layoutRoot->Children->Add(hoverBorder);

        try
        {
            onRenderCallback = gcnew RenderCallbackManaged(this, &Viewport::UpdateImage);
            auto unmanagedRenderCallback = static_cast<RenderCallbackUnmanaged>(
                Marshal::GetFunctionPointerForDelegate(onRenderCallback).ToPointer());
            vctx = EngineInterop::CreateView(
                (char *)bitmap->BackBuffer.ToPointer(), unmanagedRenderCallback);

            isInitialized = true;
        }
        catch (const std::runtime_error &e)
        {
            this->Background = gcnew SolidColorBrush(Colors::Red);
            std::cerr << e.what() << std::endl;
        }
        catch (...)
        {
            this->Background = gcnew SolidColorBrush(Colors::Red);
            std::cerr << "Unhandled exception thrown." << std::endl;
        }
    }

    void Viewport::OnRenderSizeChanged(SizeChangedInfo ^ info)
    {
        if (!isInitialized)
            return;

        int width = (int)Math::Max(info->NewSize.Width, 128.0);
        int height = (int)Math::Max(info->NewSize.Height, 128.0);

        bitmap = gcnew WriteableBitmap(width, height, 96, 96, PixelFormats::Pbgra32, nullptr);
        auto location = this->TranslatePoint(Point(0, 0), Application::Current->MainWindow);
        vctx->resize(
            location.X, location.Y, width, height, (char *)bitmap->BackBuffer.ToPointer());
        EngineInterop::RenderView(*vctx);
        image->Source = bitmap;
    }

    void Viewport::UpdateImage()
    {
        if (!isInitialized || bitmap == nullptr)
            return;

        bitmap->Lock();
        bitmap->AddDirtyRect(Int32Rect(0, 0, bitmap->PixelWidth, bitmap->PixelHeight));
        bitmap->Unlock();
    }

    void Viewport::OnMouseEnter(MouseEventArgs ^ args)
    {
        EngineInterop::SetViewportContextHoverState(vctx, true);
        hoverBorder->Visibility = Windows::Visibility::Visible;
    }

    void Viewport::OnMouseLeave(MouseEventArgs ^ args)
    {
        EngineInterop::SetViewportContextHoverState(vctx, false);
        hoverBorder->Visibility = Windows::Visibility::Hidden;
    }

    void Viewport::OnMouseDown(MouseButtonEventArgs ^ args)
    {
        this->Focus();
    }

    void Viewport::OnGotFocus(RoutedEventArgs ^ args)
    {
        EngineInterop::SetViewportContextFocusState(vctx, false);
        focusBorder->BorderBrush = focusedBrush;
    }

    void Viewport::OnLostFocus(RoutedEventArgs ^ args)
    {
        EngineInterop::SetViewportContextFocusState(vctx, false);
        focusBorder->BorderBrush = unfocusedBrush;
    }

    Viewport::~Viewport()
    {
        if (!isInitialized)
            return;

        isInitialized = false;
        EngineInterop::DetachView(vctx);
    }
}}}