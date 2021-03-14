#pragma once

#include <windows.h>
#include "editor.h"
#include "EditorView.h"

namespace Terrain { namespace Engine { namespace Interop {
    class ViewportContext
    {
        EditorView editorView;
        void *viewState;

        uint32 viewportWidth;
        uint32 viewportHeight;
        uint32 viewportX;
        uint32 viewportY;

    public:
        HDC deviceContext;

        ViewportContext(HDC deviceContext);

        EditorViewContext getViewContext() const;
        EditorView getEditorView() const;
        bool isDetached() const;

        void render();
        void resize(uint32 x, uint32 y, uint32 width, uint32 height);
        void setEditorView(EditorView editorView);
        void setViewState(void *viewState);

        void detach();
        void reattach(HDC deviceContext);
    };
}}}