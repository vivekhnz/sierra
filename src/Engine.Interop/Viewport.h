#pragma once

#include "ViewportContext.hpp"

namespace Terrain { namespace Engine { namespace Interop {
public
    ref class Viewport : System::Windows::Controls::UserControl
    {
        delegate void RenderCallbackManaged();

        ViewportContext *vctx;
        EditorView editorView;

        System::Windows::Controls::Grid ^ layoutRoot;
        System::Windows::Controls::Image ^ image;
        System::Windows::Media::Imaging::WriteableBitmap ^ bitmap;
        System::Windows::Controls::Border ^ hoverBorder;

        bool isInitialized;
        bool isInDesignMode;
        RenderCallbackManaged ^ onRenderCallback;

        void UpdateImage();
        void OnViewUpdated();

    public:
        Viewport();
        ~Viewport();

        property EditorView View
        {
            EditorView get()
            {
                return editorView;
            }

            void set(EditorView value)
            {
                editorView = value;
                OnViewUpdated();
            }
        }

        void OnRenderSizeChanged(System::Windows::SizeChangedInfo ^ info) override;
        void OnMouseEnter(System::Windows::Input::MouseEventArgs ^ args) override;
        void OnMouseLeave(System::Windows::Input::MouseEventArgs ^ args) override;
        void OnMouseDown(System::Windows::Input::MouseButtonEventArgs ^ args) override;
    };
}}}