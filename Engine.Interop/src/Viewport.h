#pragma once

#include "ViewportContext.hpp"

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Controls;
using namespace System::Windows::Input;
using namespace System::Windows::Media;
using namespace System::Windows::Media::Imaging;
using namespace System::Windows::Threading;

namespace Terrain { namespace Engine { namespace Interop {
public
    ref class Viewport : UserControl
    {
        delegate void RenderCallbackManaged();

        ViewportContext *vctx;
        Worlds::ViewportWorld world;

        Grid ^ layoutRoot;
        Image ^ image;
        WriteableBitmap ^ bitmap;
        Border ^ focusBorder;
        Border ^ hoverBorder;

        bool isInitialized;
        bool isInDesignMode;
        SolidColorBrush ^ unfocusedBrush;
        SolidColorBrush ^ focusedBrush;
        RenderCallbackManaged ^ onRenderCallback;

        void UpdateImage();
        void OnWorldUpdated();

    public:
        Viewport();
        ~Viewport();

        property Worlds::ViewportWorld World
        {
            Worlds::ViewportWorld get()
            {
                return world;
            }

            void set(Worlds::ViewportWorld value)
            {
                world = value;
                OnWorldUpdated();
            }
        }

        void OnRenderSizeChanged(SizeChangedInfo ^ info) override;
        void OnMouseEnter(MouseEventArgs ^ args) override;
        void OnMouseLeave(MouseEventArgs ^ args) override;
        void OnMouseDown(MouseButtonEventArgs ^ args) override;
        void OnGotFocus(RoutedEventArgs ^ args) override;
        void OnLostFocus(RoutedEventArgs ^ args) override;
    };
}}}