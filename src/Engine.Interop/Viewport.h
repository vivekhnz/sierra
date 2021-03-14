#pragma once

#include "ViewportContext.hpp"

ref class ViewportHwndHost;

namespace Terrain { namespace Engine { namespace Interop {

public
    ref class Viewport : System::Windows::Controls::UserControl
    {
        ViewportContext *vctx;
        EditorView editorView;

        System::Windows::Controls::Grid ^ layoutRoot;
        ViewportHwndHost ^ hwndHost;

        bool isInitialized;

        System::Windows::FrameworkElement ^ visualParent;
        System::Windows::SizeChangedEventHandler ^ parentSizeChangedEventHandler;

        void OnLoaded(System::Object ^ sender, System::Windows::RoutedEventArgs ^ args);
        void OnParentSizeChanged(
            System::Object ^ sender, System::Windows::SizeChangedEventArgs ^ args);

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
            }
        }

        void OnRenderSizeChanged(System::Windows::SizeChangedInfo ^ info) override;
    };
}}}